// Copyright (c) 2023 Robin E. R. Davies
//
// Permission is hereby gra nted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "lvtk_ui/Lv2FileDialog.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkEditBoxElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/LvtkScrollContainerElement.hpp"
#include "lvtk/LvtkIndefiniteProgressElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkTheme.hpp"
#include "lvtk/IcuString.hpp"
#include "lvtk/LvtkLog.hpp"
#include "lvtk_ui/GlobMatcher.hpp"
#include <filesystem>
#include <algorithm>
#include "lvtk_ui/MimeTypes.hpp"
#include <cstring>
#include <mutex>
#include <thread>
#include "ss.hpp"
#include "lvtk_ui/MimeTypes.hpp"

#define XK_MISCELLANY
#include <X11/keysymdef.h>

#ifdef __linux__
#include <sys/stat.h>
#endif

using namespace lvtk::ui;
using namespace lvtk;

#if defined(__linux__)
static bool isExecutable(const std::filesystem::path &path)
{
    struct stat status;
    memset(&status, 0, sizeof(status));
    if (stat(path.string().c_str(), &status) != 0)
    {
        return false;
    }
    if (status.st_mode & S_ISDIR(status.st_mode))
    {
        return false;
    }
    return (status.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
}

bool IsHiddenFile(const std::filesystem::path &path)
{
    return path.filename().string().starts_with('.');
}
#elif defined(__win32__)
static bool isExecutable(const std::filesystem::path &path)
{
    std::string extension = path.extension();
    return extension == ".exe" || extension == ".EXE";
}
bool IsHiddenFile(const std::filesystem::path &path)
{
    FIX ME.
}

#endif

namespace lvtk::ui
{
    // directories forbidden because they are dangerous, infested with symlinks, and/or just plain uninteresting for practical searches.

#ifdef __linux__
    static const std::vector<std::filesystem::path> forbiddenDirectories{
        "/dev", "/sys", "/proc", "/snap", "/run", "/tmp", "/boot", "/root", "/lost+found", "/var/run", "/var/tmp", "/var/cache"};

    static bool IsForbiddenDirectory(const std::filesystem::path &path)
    {
        for (auto &directory : forbiddenDirectories)
        {
            if (path == directory)
            {
                return true;
            }
        }
        return false;
    }
#else
    static bool IsForbiddenDirectory(const std::filesystem::path &path)
    {
        return false;
    }
#endif
}

static std::filesystem::path ConvertHomePath(const std::string &path)
{
    if (path.length() >= 1)
    {
        if (path == "~")
        {
            std::filesystem::path homeDirectory = std::filesystem::path(std::getenv("HOME"));
            return homeDirectory;
        }
        if (path[0] == '~' && (path[1] == std::filesystem::path::preferred_separator || path[1] == '/'))
        {
            std::filesystem::path homeDirectory = std::filesystem::path(std::getenv("HOME"));
            return homeDirectory / path.substr(2);
        }
    }
    return path;
}

static constexpr size_t MAX_MATCHES = 200;

class Lv2FileDialog::SearchTask : public LvtkObject
{
    SearchTask()
    {
    }

public:
    using CallbackT = std::function<void(const std::vector<std::string> &results, SearchStatus status)>;
    SearchTask(
        Lv2FileDialog *dlg,
        const std::string &path,
        const std::string &searchString,
        std::function<void(const std::vector<std::string> &results, SearchStatus status)> callback)
        : dlg(dlg), path(path), searchString(searchString), callback(callback), icuString(IcuString::Instance())
    {
        lastUpdateTime = animation_clock_t::now();

        globMatcher.SetPattern(searchString);
        this->thread = std::make_unique<std::thread>([this]()
                                                     { ThreadProc(); });
    }
    ~SearchTask()
    {
        Cancel();
        if (thread)
        {
            thread->join();
            thread = nullptr;
        }
    }
    void Cancel()
    {
        std::lock_guard lock{mutex};
        canceled = true;
        if (postResultHandle)
        {
            dlg->CancelPostDelayed(postResultHandle);
            postResultHandle = AnimationHandle::InvalidHandle;
        }
    }
    bool Canceled()
    {
        std::lock_guard lock{mutex};
        return canceled;
    }

    class CanceledException : std::exception
    {
        CanceledException() : std::exception()
        {
        }
        virtual const char *what() const noexcept override { return "Cancelled"; }
    };

    static std::vector<std::string> Filter(Lv2FileDialog *dlg, const std::vector<std::string> &input, const std::string &filter)
    {
        SearchTask searchTask;
        searchTask.globMatcher.SetPattern(filter);

        for (auto &s : input)
        {
            std::filesystem::path path(s);
            MatchScore score = searchTask.GlobMatch(path.filename(), s, filter);
            if (score != MatchScore::NoMatch)
            {
                if (dlg->FileTypeMatch(path))
                {
                    searchTask.result.push_back(SearchResult{score, path});
                }
            }
        }
        // rely on stable sort to preserve original order as a secondary key.
        std::sort(
            searchTask.result.begin(), searchTask.result.end(),
            [](const SearchResult &left, const SearchResult &right)
            {
                if (left.matchScore != right.matchScore)
                {
                    return left.matchScore < right.matchScore;
                }
                return false;
            });
        std::vector<std::string> result;
        result.reserve(searchTask.result.size());
        for (auto &v : searchTask.result)
        {
            result.push_back(v.path);
        }
        return result;
    }
    void SortResults()
    {
        std::sort(result.begin(), result.end(),
                  [this](const SearchResult &left, const SearchResult &right)
                  {
                      if (left.matchScore != right.matchScore)
                      {
                          return left.matchScore < right.matchScore;
                      }
                      return icuString->collationCompare(left.path, right.path) < 0;
                  });
        // marshal onto UI thread across a thread boundary/
        if (result.size() > MAX_MATCHES)
        {
            result.resize(MAX_MATCHES);
        }
    }
    void PostResult(SearchStatus status)
    {
        lastUpdateTime = animation_clock_t::now();
        std::lock_guard guard{mutex}; // guarding postResultHandle specifically.
        {
            if (canceled)
                return;

            SortResults();

            // shared_ptr allows the capture variable to be copied and moved painlessly.
            std::shared_ptr<std::vector<std::string>> data = std::make_shared<std::vector<std::string>>();
            data->reserve(result.size());

            for (auto &r : result)
            {
                data->push_back(r.path);
            }

            postResultHandle = dlg->PostDelayed(
                0,
                [this, data = std::move(data), status]()
                {
                    try
                    {
                        this->CheckValid();
                        callback(*(data.get()), status);
                    }
                    catch (std::exception &e)
                    {
                        LogError("Lifetime violation on Lv2FileDialog::PostDelayed of SearchTask results.");
                    }
                });
        }
    }

    int modUpdateCount = 0;
    animation_clock_t::time_point lastUpdateTime;
    bool CheckForUpdateOrCancel()
    {
        if (canceled)
        {
            return true;
        }
        if (++modUpdateCount >= 100)
        {
            using namespace std::chrono;
            modUpdateCount = 0;
            if (animation_clock_t::now() - lastUpdateTime > duration_cast<animation_clock_t::duration>(2000ms))
            {
                PostResult(SearchStatus::Interrim);
            }
        }

        return false;
    }
    static const std::vector<std::filesystem::path> forbiddenDirectories;

    static bool IsParent(const std::filesystem::path &parent, const std::filesystem::path &child)
    {
        auto iParent = parent.begin();
        auto iChild = child.begin();
        while (iParent != parent.end())
        {
            if (iChild == child.end())
            {
                return false;
            }
            if ((*iParent) != (*iChild))
            {
                return false;
            }

            ++iParent;
            ++iChild;
        }
        return true;
    }
    int symLinkLevel = 0;
    static constexpr int MAX_SYM_LINK_LEVEL = 4;

private:
    IcuString::Ptr icuString;

    void ThreadProc()
    {
        try
        {
            Search(this->path);
        }
        catch (const CanceledException &e)
        {
        }
        if (!Canceled())
        {
            PostResult(SearchStatus::Complete);
        }
    }

public:
    GlobMatcher globMatcher;

    enum class MatchScore
    {
        ExactMatch = 1,         // exact match of a file.
        SubdirectoryMatch,      // exact match of a filename in a subdirectory.
        MatchStart,             // matches the start of a filename in the search directory.,
        SubdirectoryMatchStart, // matches the start of a filename in the search directory.,
        MatchDirectory,
        NoMatch
    };
    struct SearchResult
    {
        MatchScore matchScore;
        std::filesystem::path path;
    };

    bool Search(const std::filesystem::path &path)
    {
        if (IsForbiddenDirectory(path))
        {
            return true;
        }
        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(path))
            {
                if (entry.is_regular_file() && !IsHiddenFile(entry.path()))
                {
                    MatchScore score = Matches(entry);
                    if (score != MatchScore::NoMatch)

                    {
                        result.push_back({score, entry.path().string()});
                        if (result.size() == MAX_MATCHES)
                        {
                            return false;
                        }
                    }
                }
                if (CheckForUpdateOrCancel())
                {
                    return false;
                }
            }
            for (const auto &entry : std::filesystem::directory_iterator(path))
            {
                if (entry.is_directory())
                {
                    if (IsHiddenFile(entry.path()))
                    {
                        continue;
                    }
                    try
                    {
                        if (entry.is_symlink())
                        {
                            auto canonicalChild = std::filesystem::canonical(entry.path());
                            auto canonicalPath = std::filesystem::canonical(path);
                            if (IsParent(canonicalChild, canonicalPath))
                            {
                                continue;
                            }
                            ++symLinkLevel;
                            if (symLinkLevel < MAX_SYM_LINK_LEVEL)
                            {
                                try
                                {
                                    if (!Search(entry.path()))
                                    {
                                        --symLinkLevel;
                                        return false;
                                    }
                                }
                                catch (const std::exception &)
                                {
                                }
                            }
                            --symLinkLevel;
                        }
                        else
                        {
                            if (!Search(entry.path()))
                            {
                                return false;
                            }
                        }
                        if (CheckForUpdateOrCancel())
                        {
                            return false;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        LogDebug(SS("Search: " << e.what() << "(" << entry.path() << ")"));
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            LogDebug(SS("Search: " << e.what() << "(" << path << ")"));
        }
        return true;
    }

    MatchScore Matches(const std::filesystem::directory_entry &dirEntry)
    {
        std::filesystem::path path = dirEntry.path();
        if (!dirEntry.is_directory() && dlg->FileTypeMatch(path) && !IsHiddenFile(path))
        {
            std::string extra = dirEntry.path().string().substr(this->path.length() + 1);
            std::filesystem::path extraPath(extra);

            MatchScore result = GlobMatch(path.filename(), extra, this->searchString);
            return result;
        }
        else
        {
            return MatchScore::NoMatch;
        }
    }

    MatchScore GlobMatch(const std::string &fileName, const std::string &relativePath, const std::string &pattern)
    {
        if (fileName == pattern)
        {
            if (fileName == relativePath)
            {
                return MatchScore::ExactMatch;
            }
            else
            {
                return MatchScore::SubdirectoryMatch;
            }
        }
        if (fileName.starts_with(pattern))
        {
            if (fileName == relativePath)
            {
                return MatchScore::MatchStart;
            }
            else
            {
                return MatchScore::SubdirectoryMatchStart;
            }
        }
        if (globMatcher.Matches(fileName))
        {
            if (fileName == relativePath)
            {
                return MatchScore::ExactMatch;
            }
            else
            {
                return MatchScore::SubdirectoryMatch;
            }
        }
        if (globMatcher.Matches(relativePath))
        {
            return MatchScore::SubdirectoryMatch;
        }
        return MatchScore::NoMatch;
    }

    Lv2FileDialog *dlg;
    AnimationHandle postResultHandle;
    const std::string path;
    std::string searchString;

    std::vector<SearchResult> result;
    std::function<void(const std::vector<std::string> &results, SearchStatus status)> callback;

    std::unique_ptr<std::thread> thread;
    bool canceled = false;
    std::mutex mutex;
};

Lv2FileDialog::Lv2FileDialog(const std::string &title, const std::string &settingsKey)
    : icuString(IcuString::Instance())
{
    using namespace std::chrono;

    panels = gPanels;
    SettingsKey(settingsKey);
    SelectedLocationProperty.SetElement(this, &Lv2FileDialog::OnSelectedLocationChanged);
    SelectedFileProperty.SetElement(this, &Lv2FileDialog::OnSelectedFileChanged);
    SelectedFileTypeProperty.SetElement(this, &Lv2FileDialog::OnSelectedFileTypeChanged);
    FileTypesProperty.SetElement(this, &Lv2FileDialog::OnFileTypesChanged);

    DefaultSize(LvtkSize(800, 600));
    MinSize(LvtkSize(600, 400));
    MaxSize(LvtkSize(4096, 4096));
    Title(title);
}

void Lv2FileDialog::Show(LvtkWindow *parent)
{
    Settings(parent->Settings());
    FileLocation location = LoadSettings();

    super::Show(parent);

    using namespace std::chrono;

    searchBarAnimator.Initialize(this->searchBar.get(), 120ms, 120ms,
                                 [this](double animationValue)
                                 {
                                     OnSearchBarAnimate(animationValue);
                                 });

    Navigate(location);
    SelectPanel(location);
    LoadBreadcrumbBar();
    ClearBackList();
}

LvtkElement::ptr Lv2FileDialog::RenderBreadcrumb(
    const std::string &icon,
    const std::string &label,
    const std::string &path)
{
    if (currentLocation.locationType == LocationType::None)
    {
        return LvtkElement::Create();
    }
    auto container = LvtkButtonBaseElement::Create();
    std::string navPath = path;
    container->Clicked.AddListener(
        [this, navPath](const LvtkMouseEventArgs &args)
        {
            this->OnFileSelected(navPath,args);
            return true;
        });
    container->Style()
        .Padding({4});

    {

        auto typography = LvtkTypographyElement::Create();
        typography->Text(label).Variant(
            path == currentLocation.path
                ? LvtkTypographyVariant::BodyPrimary
                : LvtkTypographyVariant::BodySecondary);
        typography->Style().SingleLine(true);
        auto flexGrid = LvtkFlexGridElement::Create();
        flexGrid->Style()
            .FlexAlignItems(LvtkAlignment::Center)
            .ColumnGap(4);
        if (icon.length() != 0)
        {
            auto element = LvtkSvgElement::Create();
            element->Source(icon);
            element->Style()
                .Width(20)
                .Height(20)
                .HorizontalAlignment(LvtkAlignment::Start)
                .VerticalAlignment(LvtkAlignment::Center)
                .TintColor(Theme().secondaryTextColor);
            flexGrid->AddChild(element);
        }
        else
        {
            // spacer.
            auto element = LvtkElement::Create();
            element->Style()
                .Width(0)
                .Height(20);
            container->Style()
                .MaxWidth(LvtkMeasurement::Percent(60))
                .Padding({0, 4, 0, 4});
            typography->Style()
                .Ellipsize(LvtkEllipsizeMode::Center)
                .Padding({4, 0, 4, 0});
            flexGrid->AddChild(element);
        }
        {
            flexGrid->AddChild(typography);
        }
        container->AddChild(flexGrid);
    }
    return container;
}

std::vector<Lv2FileDialog::Breadcrumb> Lv2FileDialog::GetBreadcrumbs(FilePanel &panel, const std::filesystem::path &path_)
{
    std::vector<Breadcrumb> result;
    std::filesystem::path path = ConvertHomePath(path_);

    if (panel.locationType == LocationType::Path)
    {
        std::filesystem::path pathResult = std::filesystem::path();

        std::filesystem::path parentPath = ConvertHomePath(panel.path);
        auto parentIter = parentPath.begin();
        auto sourceIter = path.begin();
        while (parentIter != parentPath.end())
        {
            if (sourceIter == path.end())
            {
                break;
            }
            if (parentIter->string() != sourceIter->string())
            {
                break;
            }
            pathResult = pathResult / (*sourceIter);
            ++parentIter;
            ++sourceIter;
        }
        while (sourceIter != path.end())
        {
            pathResult /= *sourceIter;
            result.push_back({sourceIter->string(), pathResult.string()});
            ++sourceIter;
        }
    }
    else
    {
        std::filesystem::path pathResult = std::filesystem::path();

        auto iter = path.begin();

        while (iter != path.end())
        {
            pathResult = pathResult / *iter;
            result.push_back(Breadcrumb{iter->string(), pathResult.string()});
            ++iter;
        }
    }
    return result;
}

LvtkElement::ptr Lv2FileDialog::RenderSearchBar()
{
    auto sizer = LvtkContainerElement::Create();
    this->searchBarSizer = sizer;
    sizer->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .Visibility(LvtkVisibility::Collapsed);
    {
        auto container = LvtkFlexGridElement::Create();
        this->searchBar = container;
        container->Style()
            .BorderColor(Theme().secondaryTextColor)
            .FlexAlignItems(LvtkAlignment::Center)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexOverflowJustification(LvtkFlexOverflowJustification::End)
            .BorderWidth(0.25)
            .Margin({4})
            .Padding({12, 4, 4, 3})
            .RoundCorners({40});
        {
            auto icon = LvtkSvgElement::Create();
            icon->Source("FileDialog/search.svg");
            icon->Style()
                .Width(20)
                .Height(20)
                .TintColor(Theme().secondaryTextColor);
            container->AddChild(icon);
        }
        {
            auto searchEdit = LvtkEditBoxElement::Create();
            this->searchEdit = searchEdit;
            searchEdit->Variant(EditBoxVariant::Underline);
            searchEdit->Style()
                .HorizontalAlignment(LvtkAlignment::Stretch)
                .BorderWidth({0})
                .Margin({4, 2, 0, 2});

            searchTextChangedHandle = searchEdit->TextProperty.addObserver(
                [this](const std::string &)
                {
                    StartSearchTimer(false);
                    return false;
                });
            container->AddChild(searchEdit);
        }
        {
            auto element = LvtkButtonElement::Create();
            element->Icon("FileDialog/close.svg");
            element->Variant(LvtkButtonVariant::ImageButton);
            element->Style()
                .RoundCorners({0, 20, 0, 20})
                .Padding({4, 4, 4, 4})
                .Margin(0)
                .IconSize(16);
            container->AddChild(element);
            searchCancelHandle = element->Clicked.AddListener(
                [this](const LvtkMouseEventArgs &)
                {
                    CloseSearchBox();
                    return true;
                });
        }
        sizer->AddChild(container);
    }
    return sizer;
}

LvtkElement::ptr Lv2FileDialog::RenderSearchProgressBar()
{
    auto element = LvtkIndefiniteProgressElement::Create();
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch);
    SearchProgressActiveProperty.Bind(element->ActiveProperty);
    return element;
}
LvtkElement::ptr Lv2FileDialog::RenderBreadcrumbBar()
{
    auto container = LvtkFlexGridElement::Create();
    container->Style()
        .BorderColor(Theme().secondaryTextColor)
        .FlexAlignItems(LvtkAlignment::Center)
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .FlexJustification(LvtkFlexJustification::Start)
        .FlexOverflowJustification(LvtkFlexOverflowJustification::End)
        .BorderWidth(0.25)
        .Margin({4})
        .Padding({12, 4, 12, 4})
        .RoundCorners({40});

    this->breadcrumbBar = container;
    return container;
}

void Lv2FileDialog::LoadBreadcrumbBar()
{
    auto container = this->breadcrumbBar;
    container->RemoveAllChildren();

    if (currentPanel.locationType == LocationType::None)
    {
        return;
    }

    container->AddChild(RenderBreadcrumb(currentPanel.icon, currentPanel.label, currentPanel.path));
    std::string path;
    if (this->SelectedFile().length() != 0)
    {
        path = SelectedFile();
    }
    else
    {
        path = this->currentLocation.path;
    }

    std::vector<Breadcrumb> breadcrumbs = GetBreadcrumbs(currentPanel, path);

    // The first breadcrumb in favorites and recent is the root directory '/'. We want it to be
    // (subtly, subversively) an active link, so, display the '/' breadcrumb, and skip the
    // two separators on either side:
    //       X Recent / usr / lib / lv2
    // Clicking on the first (and only the first) slash will jump to  X Computer (which has a path of "/")."
    // Very subtle implementation detail, but the behaviour is (I think) correct, if not obvious.

    int seperatorSkipCount =
        (currentPanel.locationType == LocationType::Path)
            ? 0
            : 2;

    for (const auto &breadcrumb : breadcrumbs)
    {
        if (seperatorSkipCount > 0)
        {
            --seperatorSkipCount;
        }
        else
        {
            auto typography = LvtkTypographyElement::Create();
            typography->Text("/");
            typography->Variant(LvtkTypographyVariant::BodySecondary);
            typography->Style()
                .Padding({2, 2, 2, 2});
            container->AddChild(typography);
        }
        container->AddChild(RenderBreadcrumb("", breadcrumb.label, breadcrumb.path));
    }
    container->InvalidateParentLayout();
}
LvtkElement::ptr Lv2FileDialog::RenderTopPanel()
{
    auto body = LvtkFlexGridElement::Create();
    body->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .BorderWidth({0, 0, 0, 1})
        .BorderColor(Theme().dividerColor)
        .FlexAlignItems(LvtkAlignment::Center)
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .Padding({8, 4, 8, 4})
        .RowGap(8);
    {
        auto undoContainer = LvtkFlexGridElement::Create();
        {
            auto undoLocation = LvtkButtonElement::Create();
            undoLocation->Icon("undo.svg").Variant(LvtkButtonVariant::ImageButton);
            undoLocation->Style()
                .RoundCorners({20, 0, 20, 0});
            this->BackButtonEnabledProperty.Bind(undoLocation->EnabledProperty);
            undoLocation->Clicked.AddListener(
                [this](const LvtkMouseEventArgs &)
                {
                    OnNavigateBack();
                    return true;
                });
            undoContainer->AddChild(undoLocation);
        }
        {
            auto redoLocation = LvtkButtonElement::Create();
            redoLocation->Icon("redo.svg").Variant(LvtkButtonVariant::ImageButton);
            redoLocation->Style()
                .RoundCorners({0, 20, 0, 20});

            this->ForwardButtonEnabledProperty.Bind(redoLocation->EnabledProperty);

            redoLocation->Clicked.AddListener(
                [this](const LvtkMouseEventArgs &)
                {
                    OnNavigateForward();
                    return true;
                });
            undoContainer->AddChild(redoLocation);
        }
        body->AddChild(undoContainer);
    }
    {
        body->AddChild(RenderBreadcrumbBar());
    }
    {
        auto element = LvtkButtonElement::Create();
        element->Variant(LvtkButtonVariant::ImageButton)
            .Icon("FileDialog/search.svg");
        this->searchButton = element;

        element->Clicked.AddListener([this](const LvtkMouseEventArgs &)
                                     {
            OpenSearchBox();
            return true; });

        body->AddChild(element);
    }
    {
        body->AddChild(RenderSearchBar());
    }

    {
        auto element = LvtkButtonElement::Create();
        element->Variant(LvtkButtonVariant::ImageButton);
        FavoriteIconSourceProperty.Bind(element->IconProperty);
        FavoriteButtonEnabledProperty.Bind(element->EnabledProperty);
        body->AddChild(element);
        element->Clicked.AddListener(
            [this](const LvtkMouseEventArgs &)
            {
                OnToggleFavorite();
                return true;
            });
    }
    return body;
}

LvtkElement::ptr Lv2FileDialog::RenderFileList()
{
    auto container = LvtkContainerElement::Create();
    container->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);

    {
        auto scroll = LvtkScrollContainerElement::Create();
        scroll->HorizontalScrollEnabled(true)
            .VerticalScrollEnabled(false);
        scroll->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Stretch)
            .BorderWidth({0, 0, 0, 1})
            .BorderColor(Theme().dividerColor);
        FilesScrollOffsetProperty.Bind(scroll->HorizontalScrollOffsetProperty);

        {
            auto body = LvtkFlexGridElement::Create();
            this->fileListContainer = body;
            body->Style()
                .HorizontalAlignment(LvtkAlignment::Start)
                .VerticalAlignment(LvtkAlignment::Stretch)
                .Padding({8, 8, 8, 24})
                .FlexDirection(LvtkFlexDirection::Column)
                .FlexWrap(LvtkFlexWrap::Wrap)
                .ColumnGap(16)
                .RowGap(0);
            scroll->Child(body);
        }
        container->AddChild(scroll);
    }
    {
        auto element = LvtkTypographyElement::Create();
        element->Text("No files.").Variant(LvtkTypographyVariant::BodySecondary);
        element->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .TextAlign(LvtkTextAlign::Center)
            .VerticalAlignment(LvtkAlignment::Start)
            .Visibility(LvtkVisibility::Collapsed)
            .SingleLine(false)
            .MarginLeft(16)
            .MarginRight(16)
            .MarginTop(LvtkMeasurement::Percent(30));
        this->noFilesLabel = element;
        container->AddChild(element);
    }
    return container;
}

LvtkElement::ptr Lv2FileDialog::RenderSearchMessage()
{
    auto element = LvtkTypographyElement::Create();
    element->Variant(LvtkTypographyVariant::Caption);
    element->Style()
        .Padding({24, 4, 24, 4})
        .SingleLine(true)
        .Ellipsize(LvtkEllipsizeMode::End)
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .Visibility(LvtkVisibility::Collapsed);
    this->searchMessageElement = element;
    return element;
}

LvtkElement::ptr Lv2FileDialog::RenderFilePanel()
{
    auto body = LvtkVerticalStackElement::Create();
    body->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);
    body->AddChild(RenderTopPanel());

    body->AddChild(RenderSearchProgressBar());

    body->AddChild(RenderSearchMessage());
    body->AddChild(RenderFileList());
    return body;
}

namespace lvtk::ui
{
}

/*static*/
std::vector<Lv2FileDialog::FilePanel>
    Lv2FileDialog::gPanels = {
        {"Recent", "FileDialog/recent.svg", "", LocationType::Recent},
        {"Favorites", "FileDialog/favorites.svg", "", LocationType::Favorites},
        {"Home", "FileDialog/home.svg", "~", LocationType::Path},
        {"Documents", "FileDialog/documents_folder.svg", "~/Documents", LocationType::Path},
        {"Downloads", "FileDialog/download.svg", "~/Downloads", LocationType::Path},
        {"Desktop", "FileDialog/desktop_folder.svg", "~/Desktop", LocationType::Path},
        {"Music", "FileDialog/music_folder.svg", "~/Music", LocationType::Path},
        {"Pictures", "FileDialog/image_folder.svg", "~/Pictures", LocationType::Path},
        {"Videos", "FileDialog/video_folder.svg", "~/Videos", LocationType::Path},
        {"Computer", "FileDialog/hard_drive.svg", "/", LocationType::Path},

};

LvtkElement::ptr Lv2FileDialog::RenderPanel(size_t index, const FilePanel &location)
{
    auto button = LvtkButtonBaseElement::Create();
    button->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)

        ;
    {
        auto container = LvtkFlexGridElement::Create();
        container->Style()
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .FlexAlignItems(LvtkAlignment::Center)
            .ColumnGap(8)
            .Padding({16, 8, 16, 8});
        {
            auto icon = LvtkSvgElement::Create();
            icon->Source(location.icon);
            icon->Style()
                .Width(20)
                .Height(20)
                .TintColor(Theme().secondaryTextColor)
                .SingleLine(true)
                .Ellipsize(LvtkEllipsizeMode::End);
            container->AddChild(icon);
        }
        {
            auto text = LvtkTypographyElement::Create();
            text->Text(location.label).Variant(LvtkTypographyVariant::BodyPrimary);
            container->AddChild(text);
        }
        button->AddChild(container);
    }
    button->Clicked.AddListener([this, index](const LvtkMouseEventArgs &)
                                {
                          CheckValid();
                          this->OnOpenLocation((uint64_t)index);
                          return true; });
    return button;
}
LvtkElement::ptr Lv2FileDialog::RenderLocations()
{
    auto body = LvtkVerticalStackElement::Create();
    body->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .VerticalAlignment(LvtkAlignment::Stretch)
        //.Background(LvtkColor::Blend(0.003, Theme().dialogBackgroundColor, Theme().primaryTextColor))
        .BorderWidth({0, 0, 1, 0})
        .Padding({8, 8, 8, 8})
        .BorderColor(Theme().dividerColor);
    for (size_t i = 0; i < panels.size(); ++i)
    {
        auto l = RenderPanel(i, panels[i]);
        this->locations.push_back(l);
        body->AddChild(l);
    }
    return body;
}
LvtkElement::ptr Lv2FileDialog::RenderBody()
{
    auto body = LvtkFlexGridElement::Create();
    body->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
        .FlexWrap(LvtkFlexWrap::NoWrap);
    {
        body->AddChild(RenderLocations());
    }
    {
        body->AddChild(RenderFilePanel());
    }
    return body;
}

// static LvtkElement::ptr BoxWrap(LvtkElement::ptr element)
// {
//     auto container = LvtkContainerElement::Create();
//     container->Style()
//         .BorderWidth({1})
//         .BorderColor(LvtkColor(1,0.5,0.5))
//         ;
//     container->AddChild(element);
//     return container;
// }

LvtkElement::ptr Lv2FileDialog::RenderFooter()
{
    auto footer = LvtkFlexGridElement::Create();
    footer->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::End)
        .FlexJustification(LvtkFlexJustification::End)
        .FlexAlignItems(LvtkAlignment::Center)
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .ColumnGap(8)
        .Padding({20, 16, 24, 16})
        .BorderWidth({0, 1, 0, 0})
        .BorderColor(Theme().dividerColor);
    {
        auto element = LvtkTypographyElement::Create();
        element->Text("Filter: ").Variant(LvtkTypographyVariant::BodySecondary);
        element->Style()
            .SingleLine(true)
            .Padding({4});
        footer->AddChild((element));
    }
    {
        auto element = LvtkDropdownElement::Create();
        this->SelectedFileTypeProperty.Bind(element->SelectedIdProperty);
        this->FileTypeDropdownItemsProperty.Bind(element->DropdownItemsProperty);
        footer->AddChild((element));
    }
    {
        auto element = LvtkElement::Create();
        element->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .Height(1);
        footer->AddChild(element);
    }
    {
        LvtkButtonElement::ptr button = LvtkButtonElement::Create();
        button->Style()
            .Width(100);
        this->cancelButton = button;
        button->Variant(LvtkButtonVariant::BorderButton);
        button->Text("Cancel");
        cancelEventHandle = button->Clicked.AddListener(
            [this](const LvtkMouseEventArgs &)
            {
                CheckValid();
                OnCancel();
                return true;
            });
        footer->AddChild((button));
    }
    {
        LvtkButtonElement::ptr button = LvtkButtonElement::Create();
        button->Style()
            .Width(100);
        this->okButton = button;
        button->Variant(LvtkButtonVariant::BorderButtonDefault);
        button->Text("OK");

        this->OkEnabledProperty.Bind(button->EnabledProperty);

        okEventHandle = button->Clicked.AddListener(
            [this](const LvtkMouseEventArgs &)
            {
                CheckValid();
                OnOk();
                return true;
            });
        footer->AddChild((button));
    }
    return footer;
}
LvtkElement::ptr Lv2FileDialog::Render()
{
    auto container = LvtkVerticalStackElement::Create();
    container->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
        .Background(Theme().dialogBackgroundColor);
    {
        {
            container->AddChild(RenderBody());
        }

        {
            container->AddChild(RenderFooter());
        }
    }
    OnOpenLocation(SelectedLocation());
    return container;
}

void Lv2FileDialog::OnOk()
{
    if (SelectedFile().length() == 0)
    {
        return;
    }

    std::string recentEntry = this->SelectedFile();
    for (auto i = recentEntries.begin(); i != recentEntries.end(); ++i)
    {
        if ((*i) == recentEntry)
        {
            recentEntries.erase(i);
            break;
        }
    }
    recentEntries.insert(recentEntries.begin(), recentEntry);

    SaveSettings();
    okClose = true;
    Close();
}

void Lv2FileDialog::OnCancel()
{
    Close();
}

void Lv2FileDialog::OnSelectedLocationChanged(int64_t value)
{
    for (size_t i = 0; i < locations.size(); ++i)
    {
        auto &location = locations[i];
        if (i == (size_t)value)
        {
            location->HoverState(location->HoverState() + LvtkHoverState::Selected);
        }
        else
        {
            location->HoverState(location->HoverState() - LvtkHoverState::Selected);
        }
    }
}

struct Lv2FileDialog::LvtkDialogFile
{
    LvtkDialogFile(const std::filesystem::directory_entry &dirEntry)
        : isDirectory(dirEntry.is_directory()), path(dirEntry.path())
    {
        label = path.filename();
        fileSize = 0;
        if (isDirectory)
        {
            fileSize = 0;
        }
        else if (dirEntry.is_regular_file())
        {
            fileSize = dirEntry.file_size();
        }
        lastModified = dirEntry.last_write_time();
    }
    bool isDirectory;
    std::filesystem::path path;
    std::string label;
    uintmax_t fileSize;
    std::filesystem::file_time_type lastModified;
};

std::string Lv2FileDialog::GetIcon(const LvtkDialogFile &file)
{
    if (file.isDirectory)
    {
        return "FileDialog/folder.svg";
    }
    else
    {
        return GetIcon(file.path);
    }
}

std::string Lv2FileDialog::GetIcon(const std::filesystem::path &path)
{

    if (isExecutable(path))
    {
        return "FileDialog/program.svg";
    }
    std::string mimeType = MimeTypes::MimeTypeFromPath(path);

    if (mimeType.starts_with("image/"))
    {
        return "FileDialog/image_file.svg";
    }
    if (mimeType.starts_with("audio/"))
    {
        return "FileDialog/audio_file.svg";
    }
    if (mimeType.starts_with("video/"))
    {
        return "FileDialog/video_file.svg";
    }
    if (mimeType.starts_with("text/"))
    {
        return "FileDialog/text_file.svg";
    }
    return "FileDialog/document_file.svg";
}

void Lv2FileDialog::LoadMixedDirectoryFiles(const std::vector<std::string> &files)
{

    for (auto &file : files)
    {
        std::filesystem::path path{file};

        auto buttonBase = LvtkButtonBaseElement::Create();
        buttonBase->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .MinWidth(200);
        if (file == SelectedFile())
        {
            buttonBase->HoverState(buttonBase->HoverState() + LvtkHoverState::Selected);
        }
        {
            auto container = LvtkFlexGridElement::Create();
            container->Style()
                .FlexWrap(LvtkFlexWrap::NoWrap)
                .ColumnGap(8)
                .FlexAlignItems(LvtkAlignment::Start)
                .Padding({8, 4, 8, 4});
            {
                {
                    auto icon = LvtkSvgElement::Create();
                    std::string iconPath = GetIcon(path);
                    icon->Source(iconPath);
                    icon->Style()
                        .Width(24)
                        .Height(24)
                        .TintColor(Theme().secondaryTextColor);
                    container->AddChild(icon);
                }
                {
                    auto stack = LvtkFlexGridElement::Create();
                    stack->Style()
                        .FlexDirection(LvtkFlexDirection::Column)
                        .FlexWrap(LvtkFlexWrap::NoWrap);
                    {
                        auto text = LvtkTypographyElement::Create();
                        text->Text(path.filename().string()).Variant(LvtkTypographyVariant::BodyPrimary);

                        text->Style()
                            .SingleLine(true)
                            .Ellipsize(LvtkEllipsizeMode::Center)
                            .Padding({0, 2, 0, 4});
                        stack->AddChild(text);
                    }
                    {
                        auto text = LvtkTypographyElement::Create();
                        text->Text(path.parent_path().string()).Variant(LvtkTypographyVariant::BodySecondary);
                        text->Style()
                            .SingleLine(true)
                            .Ellipsize(LvtkEllipsizeMode::Start);
                        stack->AddChild(text);
                    }
                    container->AddChild(stack);
                }
                {
                    auto favoriteIcon = LvtkSvgElement::Create();
                    favoriteIcon->Style()
                        .Width(20)
                        .Height(20)
                        .Padding({0})
                        .TintColor(Theme().secondaryTextColor);
                    if (IsFavorite(file))
                    {
                        favoriteIcon->Source("FileDialog/favorites.svg");
                    }
                    else
                    {
                        favoriteIcon->Source("blank.svg");
                    }
                    container->AddChild(favoriteIcon);
                }
            }
            buttonBase->AddChild(container);
            std::filesystem::path filePath = file;
            buttonBase->Clicked.AddListener(
                [this, filePath](const LvtkMouseEventArgs &eventArgs)
                {
                    CheckValid();
                    OnFileSelected(filePath,eventArgs);
                    return true;
                });
        }

        fileListContainer->AddChild(buttonBase);
    }
}

void Lv2FileDialog::LoadFiles(const std::filesystem::path &path)
{
    using namespace std::filesystem;

    std::vector<LvtkDialogFile> files;

    errno = 0;
    try
    {
        for (auto &dirEntry : std::filesystem::directory_iterator(path))
        {

            const auto &path = dirEntry.path();
            if (IsForbiddenDirectory(path))
            {
                continue;
            }
            if (dirEntry.is_directory())
            {
                if (!IsHiddenFile(path))
                {

                    files.push_back(LvtkDialogFile(dirEntry));
                }
            }
            else if (dirEntry.is_regular_file())
            {
                if (!IsHiddenFile(path))
                {
                    if (this->FileTypeMatch(path))
                    {
                        files.push_back(LvtkDialogFile(dirEntry));
                    }
                }
            }
        }
    }
    catch (std::exception &e)
    {
        std::string error;
        if (errno)
        {
            error = strerror(errno);
        }
        else
        {
            error = e.what();
        }
        this->noFilesLabel->Text(error);
        this->noFilesLabel->Style()
            .Visibility(LvtkVisibility::Visible);
        return;
    }

    if (files.size() == 0)
    {

        this->noFilesLabel->Style().Visibility(LvtkVisibility::Visible);
        this->noFilesLabel->Text("No files.");
    }
    else
    {
        this->noFilesLabel->Style().Visibility(LvtkVisibility::Collapsed);
    }
    std::sort(files.begin(), files.end(),
              [this](const LvtkDialogFile &a, const LvtkDialogFile &b)
              {
                  if (a.isDirectory != b.isDirectory)
                  {
                      return a.isDirectory;
                  }
                  return icuString->collationCompare(a.label, b.label) < 0;
              });

    for (auto &file : files)
    {
        auto buttonBase = LvtkButtonBaseElement::Create();
        buttonBase->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch);
        if (file.path.string() == SelectedFile())
        {
            buttonBase->HoverState(buttonBase->HoverState() + LvtkHoverState::Selected);
        }
        {
            // minimum width.
            auto spacer = LvtkElement::Create();
            spacer->Style()
                .Width(200)
                .Height(0);
            buttonBase->AddChild(spacer);
        }
        {
            auto container = LvtkFlexGridElement::Create();
            container->Style()
                .FlexWrap(LvtkFlexWrap::NoWrap)
                .ColumnGap(8)
                .FlexAlignItems(LvtkAlignment::Center)
                .Padding({8, 4, 8, 4});
            {
                {
                    auto icon = LvtkSvgElement::Create();
                    std::string iconPath = GetIcon(file);
                    icon->Source(iconPath);
                    icon->Style()
                        .Width(24)
                        .Height(24)
                        .TintColor(Theme().secondaryTextColor);
                    container->AddChild(icon);
                }
                {
                    auto text = LvtkTypographyElement::Create();
                    text->Text(file.label).Variant(LvtkTypographyVariant::BodyPrimary);
                    text->Style()
                        .SingleLine(true)
                        .Ellipsize(LvtkEllipsizeMode::Center);
                    container->AddChild(text);
                }
                {
                    auto favoriteIcon = LvtkSvgElement::Create();
                    favoriteIcon->Style()
                        .Width(20)
                        .Height(20)
                        .Padding({0})
                        .TintColor(Theme().secondaryTextColor);
                    if (IsFavorite(file.path))
                    {
                        favoriteIcon->Source("FileDialog/favorites.svg");
                    }
                    else
                    {
                        favoriteIcon->Source("blank.svg");
                    }
                    container->AddChild(favoriteIcon);
                }
            }
            buttonBase->AddChild(container);
            std::filesystem::path filePath = file.path;
            buttonBase->Clicked.AddListener(
                [this, filePath](const LvtkMouseEventArgs &eventArgs)
                {
                    CheckValid();
                    OnFileSelected(filePath,eventArgs);
                    return true;
                });
        }

        fileListContainer->AddChild(buttonBase);
    }
}

void Lv2FileDialog::LoadSearchResults()
{

    std::vector<LvtkElement::ptr> noChildren;
    fileListContainer->Children(noChildren);

    if (currentSearchStatus == SearchStatus::Idle)
    {
        SetNoFilesLabel("");
        SetSearchStatusMessage(" ");
        return;
    }
    if (currentSearchResults.size() == 0)
    {
        if (currentSearchStatus == SearchStatus::Complete)
        {
            SetNoFilesLabel("No files found.");
            SetSearchStatusMessage(" ");
            return;
        }
        else
        {
            using namespace std::chrono;
            if (animation_clock_t::now() - searchStartTime > duration_cast<animation_clock_t::duration>(2000ms))
            {
                SetNoFilesLabel("Searching...");
                SetSearchStatusMessage(" ");
            }
            else
            {
                SetNoFilesLabel("");
                SetSearchStatusMessage(" ");
            }
            return;
        }
    }
    SetNoFilesLabel("");

    if (currentSearchResults.size() >= MAX_MATCHES)
    {
        if (currentSearchStatus != SearchStatus::Complete)
        {
            SetSearchStatusMessage(SS("First " << MAX_MATCHES << " matches shown..."));
        }
        else
        {
            SetSearchStatusMessage(SS("First " << MAX_MATCHES << " matches shown."));
        }
    }
    else if (currentSearchResults.size() > 1)
    {
        if (currentSearchStatus != SearchStatus::Complete)
        {
            SetSearchStatusMessage(SS(currentSearchResults.size() << " files found..."));
        }
        else
        {
            SetSearchStatusMessage(SS(currentSearchResults.size() << " files found."));
        }
    }
    else
    {
        if (currentSearchStatus != SearchStatus::Complete)
        {
            SetSearchStatusMessage("1 file found...");
        }
        else
        {
            SetSearchStatusMessage("1 file found.");
        }
    }

    LoadMixedDirectoryFiles(currentSearchResults);
}

void Lv2FileDialog::LoadSearchResults(const std::vector<std::string> &files, SearchStatus status)
{
    this->currentSearchResults = files;
    this->currentSearchStatus = status;
    LoadSearchResults();
}

void Lv2FileDialog::LoadFileList()
{

    if (!fileListContainer)
    {
        return;
    }
    std::vector<LvtkElement::ptr> noChildren;
    fileListContainer->Children(noChildren);

    // unmount all the existing children.
    switch (currentLocation.locationType)
    {
    case LocationType::None:
        break;
    case LocationType::Path:
        LoadFiles(ConvertHomePath(this->currentLocation.path));
        break;
    case LocationType::Favorites:
    {
        LoadFavorites();
        break;
    }
    case LocationType::Recent:
    {
        LoadRecent();
        break;
    }
    }
    fileListContainer->InvalidateParentLayout();
}

std::vector<std::string> Lv2FileDialog::GetFavoritesVector()
{
    std::vector<std::string> files;
    files.reserve(this->favorites.size());
    for (const auto &v : this->favorites)
    {
        // if (this->FileTypeMatch(v)) - a favorite is a favorite regardless of file type.
        {
            files.push_back(v);
        }
    }
    std::sort(files.begin(), files.end(), [this](const std::string &a, const std::string &b)
              {
            // massive amount of conversion from utf8 to uft16 and back here. 
            // if this ever becomes a problem, convert the array to u16string before sorting.
            return icuString->collationCompare(a, b) < 0; });
    return files;
}

void Lv2FileDialog::LoadFavorites()
{
    std::vector<std::string> files = GetFavoritesVector();
    if (files.size() != 0)
    {
        SetNoFilesLabel("");
        LoadMixedDirectoryFiles(files);
    }
    else
    {
        SetNoFilesLabel("Files selected as favorites will appear here.");
    }
}

void Lv2FileDialog::LoadRecent()
{
    if (this->recentEntries.size() != 0)
    {
        SetNoFilesLabel("");
        LoadMixedDirectoryFiles(this->recentEntries);
    }
    else
    {
        SetNoFilesLabel("Recently selected files will appear here.");
    }
}

bool Lv2FileDialog::IsFavorite(const std::string &fileName)
{
    return favorites.contains(fileName);
}

void Lv2FileDialog::Navigate(const FileLocation &location)
{
    currentLocation = location;

    CloseSearchBox(false);

    SelectedFile("");
    LoadFileList();
}

void Lv2FileDialog::Navigate(const std::filesystem::path &path_)
{
    std::filesystem::path path = path_;

    if (std::filesystem::is_directory(path))
    {

        CloseSearchBox(false);

        AddCurrentLocationToBackList();
        ClearForwardList();
        FileLocation location{path, LocationType::Path};
        currentLocation = location;
        SelectedFile("");
        SelectPanel(location);
        FilesScrollOffset(0);
        LoadFileList();
        LoadBreadcrumbBar();
    }
    else
    {
        CloseSearchBox(false);

        SelectedFile(path.string());
        LoadFileList(); // could do this directly.
        LoadBreadcrumbBar();
    }
}

void Lv2FileDialog::AddCurrentLocationToBackList()
{
    {
        navigateBackList.push_back(currentLocation);
        if (navigateBackList.size() > 100)
        {
            navigateBackList.erase(navigateBackList.begin());
        }
    }
    BackButtonEnabled(navigateBackList.size() != 0);
}

void Lv2FileDialog::ClearForwardList()
{
    navigateForwardList.clear();
    ForwardButtonEnabled(false);
}
void Lv2FileDialog::ClearBackList()
{
    this->navigateBackList.clear();
    BackButtonEnabled(false);
}

void Lv2FileDialog::AddCurrentLocationToForwardList()
{
    if (currentLocation.locationType != LocationType::None)
    {
        FileLocation location = (this->currentLocation);
        navigateForwardList.push_back(location);
    }
    ForwardButtonEnabled(navigateForwardList.size() != 0);
}

void Lv2FileDialog::OnOpenLocation(int64_t locationIndex)
{

    auto &panel = panels[locationIndex];
    if (currentPanel == panel)
    {
        SelectedFile("");
        LoadBreadcrumbBar();
        return;
    }
    currentPanel = panel;
    AddCurrentLocationToBackList();
    ClearForwardList();
    FileLocation fileLocation{panel.path, panel.locationType};
    FilesScrollOffset(0);
    Navigate(fileLocation);
    SelectedLocation(locationIndex);
    LoadBreadcrumbBar();
}

static bool isParentDirectory(const std::filesystem::path &directory, std::filesystem::path path)
{
    while (true)
    {
        if (directory == path)
            return true;
        auto parent = path.parent_path();
        if (parent.empty())
        {
            break;
        }
        // parent.empty() is broken on GCC. Use this as a workaround.
        if (parent == path)
        {
            break;
        }
        path = parent;
    }
    return false;
}

void Lv2FileDialog::OnFileSelected(const std::filesystem::path &path_,const LvtkMouseEventArgs&args)
{
    std::filesystem::path path = ConvertHomePath(path_); // contents of path_ may be deleted.
    Navigate(path);

    if (!std::filesystem::is_directory(path))
    {
        auto now = clock_t::now();
        std::chrono::duration<double> duration = now-lastFileClickTime;
        lastFileClickTime = now;
        constexpr double DOUBLE_CLICK_TIME_S = 0.3;
        if (duration.count() < DOUBLE_CLICK_TIME_S
        && LvtkPoint::Distance(lastFileClickPoint,args.screenPoint) < 4)
        {
            this->OnOk();
        }
        lastFileClickPoint = args.screenPoint;
    }
}

void Lv2FileDialog::SelectPanel(const FileLocation &newLocation)
{
    size_t locationIndex = (size_t)-1;
    size_t longestMatch = 0;

    for (size_t i = 0; i < panels.size(); ++i)
    {
        auto &panel = panels[i];
        if (panel.locationType == newLocation.locationType)
        {
            if (panel.locationType == LocationType::Path)
            {
                std::filesystem::path panelPath = ConvertHomePath(panel.path);
                std::filesystem::path newLocationPath = ConvertHomePath(newLocation.path);
                if (isParentDirectory(panelPath, newLocationPath))
                {
                    size_t length = panelPath.string().length();

                    if (length > longestMatch)
                    {
                        longestMatch = length;
                        locationIndex = i;
                    }
                }
            }
            else
            {
                locationIndex = i;
                break;
            }
        }
    }
    if (locationIndex != (size_t)-1)
    {
        currentPanel = panels[locationIndex];
        SelectedLocation((int64_t)locationIndex);
    }
}

void Lv2FileDialog::OnSelectedFileChanged(const std::string &fileName)
{
    OkEnabled(fileName.length() != 0);
    if (fileName.length() != 0)
    {
        FavoriteButtonEnabled(true);
        if (IsFavorite(fileName))
        {
            FavoriteIconSource("FileDialog/favorites.svg");
        }
        else
        {
            FavoriteIconSource("FileDialog/favorite_unchecked.svg");
        }
    }
    else
    {
        FavoriteButtonEnabled(false);
        FavoriteIconSource("FileDialog/favorite_unchecked.svg");
    }
}

void Lv2FileDialog::OnNavigateBack()
{
    if (this->searchBoxOpen)
    {
        CloseSearchBox();
        return;
    }
    if (navigateBackList.size() == 0)
        return;

    AddCurrentLocationToForwardList();
    auto location = this->navigateBackList[this->navigateBackList.size() - 1];
    navigateBackList.pop_back();
    this->BackButtonEnabled(navigateBackList.size() != 0);

    Navigate(location);
    SelectPanel(location);
    LoadBreadcrumbBar();
}
void Lv2FileDialog::OnNavigateForward()
{
    if (navigateForwardList.size() == 0)
        return;

    AddCurrentLocationToBackList();
    auto location = this->navigateForwardList[this->navigateForwardList.size() - 1];
    navigateForwardList.pop_back();
    this->BackButtonEnabled(navigateBackList.size() != 0);
    this->ForwardButtonEnabled(navigateForwardList.size() != 0);

    Navigate(location);
    SelectPanel(location);
    LoadBreadcrumbBar();
}

void Lv2FileDialog::OnToggleFavorite()
{
    const std::string &selectedFile = SelectedFile();
    if (selectedFile.length() != 0)
    {
        if (IsFavorite(selectedFile))
        {
            favorites.erase(selectedFile);
            FavoriteIconSource("FileDialog/favorite_unchecked.svg");
        }
        else
        {
            favorites.insert(selectedFile);
            FavoriteIconSource("FileDialog/favorites.svg");
        }
        this->LoadFileList();
    }
}

bool Lv2FileDialog::FilePanel::operator==(const FilePanel &other) const
{
    if (locationType == LocationType::Path)
    {
        return other.locationType == locationType && other.path == path;
    }
    return other.locationType == locationType;
}
bool Lv2FileDialog::FileLocation::operator==(const FileLocation &other) const
{
    if (locationType == LocationType::Path)
    {
        return other.locationType == locationType && other.path == path;
    }
    return other.locationType == locationType;
}

void Lv2FileDialog::OnClosing()
{
    CancelSearchTimer();
    if (currentPanel.locationType != LocationType::None) // don't save if something went wrong.
    {
        SaveSettings();
    }
    searchTextChangedHandle.Release();

    super::OnClosing();

    if (okClose)
    {
        std::string path = SelectedFile();
        OK.Fire(path);
    }
    else
    {
        Cancelled.Fire();
    }
}

void Lv2FileDialog::SaveSettings()
{
    if (SettingsKey().length() == 0)
        return;

    auto settings = this->Settings();
    if (!settings.is_object())
        return;
    json_variant dialogSettings = settings["FileDialogSettings"];
    if (dialogSettings.is_null())
    {
        dialogSettings = json_variant::object();
        settings["FileDialogSettings"] = dialogSettings;
    }

    json_variant mySettings = json_variant::object();

    mySettings["recent"] = json_variant(this->recentEntries);
    mySettings["favorites"] = json_variant(this->favorites);
    mySettings["entryType"] = this->currentPanel.locationType;

    dialogSettings[SettingsKey()] = mySettings;
}
Lv2FileDialog::FileLocation Lv2FileDialog::LoadSettings()
{
    FileLocation location{"~", LocationType::Path};
    if (defaultDirectory.length() != 0)
    {
        location = FileLocation{defaultDirectory,LocationType::Path};
    }
    if (SettingsKey().length() == 0)
        return location;

    auto settings = this->Settings();
    if (!settings.is_object())
        return location;

    json_variant dialogSettings = settings["FileDialogSettings"];
    if (dialogSettings.is_null())
    {
        return location;
    }
    json_variant mySettings = dialogSettings[SettingsKey()];
    if (mySettings.is_null())
    {
        return location;
    }

    this->recentEntries = mySettings["recent"].as<std::vector<std::string>>();
    this->favorites = mySettings["favorites"].as<std::unordered_set<std::string>>();
    location.locationType = mySettings["entryType"].as<LocationType>();

    TrimNonexistentFiles();

    if (location.locationType == LocationType::Path)
    {
        location.path = "~";
        if (recentEntries.size() != 0)
        {
            std::filesystem::path directory =
                recentEntries[0];
            directory = directory.parent_path();
            if (std::filesystem::exists(directory))
            {
                location.path = directory.string();
            }
        }
    }
    else
    {
        location.path = "";
    }
    return location;
}

void Lv2FileDialog::TrimNonexistentFiles()
{
    for (auto i = recentEntries.begin(); i != recentEntries.end(); ++i)
    {
        if (!std::filesystem::exists(*i))
        {
            recentEntries.erase(i);
            --i;
        }
    }

    std::unordered_set<std::string> validFavorites;
    for (auto i = favorites.begin(); i != favorites.end(); ++i)
    {
        if (std::filesystem::exists(*i))
        {
            validFavorites.insert(*i);
        }
    }
    favorites = validFavorites;
}

bool Lv2FileDialog::OnKeyDown(LvtkKeyboardEventArgs &event)
{
    if (SearchVisible())
    {
        StartSearchTimer(false);
    }
    if (super::OnKeyDown(event))
    {
        return true;
    }
    if (event.keysymValid)
    {
        switch (event.keysym)
        {
        case XK_Escape:
        case XK_Cancel:
        {
            if (searchBoxOpen)
            {
                CloseSearchBox();
            }
            else
            {
                OnCancel();
            }
            return true;
        }
        case XK_Return:
        case XK_KP_Enter:
        {
            if (searchBoxOpen)
            {
                OnOk();
            }
            break;
        }
        default:
            break;
        }
    }
    if (event.textValid)
    {
        if (!searchBoxOpen)
        {
            if (event.keysym != XK_Return && event.keysym != XK_KP_Enter)
            {
                OpenSearchBox();
                this->searchEdit->Focus();
                this->searchEdit->Text(event.text);
                this->searchEdit->SelectEnd();
            }
        }
        return true;
    }
    return false;
}

void Lv2FileDialog::OpenSearchBox()
{
    if (searchBoxOpen)
        return;
    searchBoxOpen = true;
    searchSavedLocation = this->currentLocation;

    SearchVisible(true);
    this->currentSearchStatus = SearchStatus::Idle;
    currentSearchResults.resize(0);
    LoadSearchResults();
    searchEdit->Focus();
    StartSearchTimer(true);
}
void Lv2FileDialog::CloseSearchBox(bool restoreContent)
{
    if (!searchBoxOpen)
    {
        return;
    }
    SetNoFilesLabel("");
    searchBoxOpen = false;

    SearchVisible(false);
    SearchProgressActive(false);

    if (restoreContent)
    {
        Navigate(searchSavedLocation);
        SelectPanel(searchSavedLocation);
        LoadBreadcrumbBar();
    }
}

Lv2FileDialog &Lv2FileDialog::SearchVisible(bool visible)
{
    if (this->searchVisible != visible)
    {
        this->searchVisible = visible;
        if (searchVisible)
        {

            searchBarAnimator.SetTarget(1.0);
        }
        else
        {
            searchTask = nullptr;
            SearchProgressActive(false);
            SetSearchStatusMessage("");
            CancelSearchTimer();

            // SearchProgressActive(false);
            searchBarAnimator.SetTarget(0.0);

            if (this->searchEdit->Focused())
            {
                Focus(nullptr);
            }
        }
    }
    return *this;
}
bool Lv2FileDialog::SearchVisible() const
{
    return searchVisible;
}

void Lv2FileDialog::CancelSearchTimer()
{
    if (searchTimerHandle)
    {
        CancelPostDelayed(searchTimerHandle);
    }
}
void Lv2FileDialog::StartSearchTimer(bool noDelay)
{
    CancelSearchTimer();
    if (!this->SearchVisible())
    {
        return;
    }

    if (this->currentLocation.locationType == LocationType::Path)
    {
        // handle file search
        searchTimerHandle = this->PostDelayed(
            noDelay ? 125 : 1000, // Well. Actually, dealy until the animation completes.
            [this]()
            {
                StartSearchTask();
            });
    }
    else
    {
        // handle search for favorites and recent.
        DirectSearch();
    }
}

void Lv2FileDialog::DirectSearch()
{
    std::vector<LvtkElement::ptr> noChildren;
    fileListContainer->Children(noChildren);

    std::vector<std::string> baseList;
    switch (currentLocation.locationType)
    {
    case LocationType::Recent:
        baseList = this->recentEntries;
        break;

    case LocationType::Favorites:
        baseList = GetFavoritesVector();
        break;
    case LocationType::None:
    case LocationType::Path:
        throw std::runtime_error("Shouldn't be here.");
        break;
    }

    std::vector<std::string> result = SearchTask::Filter(this, baseList, this->searchEdit->Text());
    LoadMixedDirectoryFiles(result);
    if (result.size() == 0)
    {
        this->SetSearchStatusMessage(" ");
        this->SetNoFilesLabel("No matches found.");
        this->SelectedFile("");
    }
    else if (result.size() == 1)
    {
        this->SetSearchStatusMessage("1 match found.");
        this->SetNoFilesLabel("");
        this->SelectedFile(result[0]);
    }
    else
    {
        this->SetSearchStatusMessage(SS(result.size() << " matches found."));
        this->SetNoFilesLabel("");
        this->SelectedFile("");
    }
}

void Lv2FileDialog::StartSearchTask()
{
    if (this->currentLocation.locationType == LocationType::Path)
    {

        SelectedFile("");
        FilesScrollOffset(0);
        this->searchTask = nullptr; // join with the old searchtask if there is one.
        SearchProgressActive(false);

        this->searchTask = std::make_unique<SearchTask>(
            this,
            ConvertHomePath(currentLocation.path),
            searchEdit->Text(),
            [this](const std::vector<std::string> &results, SearchStatus status)
            {
                CheckValid();
                if (results.size() == 1 && SelectedFile() == "")
                {
                    SelectedFile(results[0]);
                }
                LoadSearchResults(results, status);
                if (status == SearchStatus::Complete)
                {
                    SearchProgressActive(false);
                }
            });
        SearchProgressActive(true);
    }
}

bool Lv2FileDialog::FileTypeMatch(const std::filesystem::path &path) const
{
    if (!currentFileFilter.has_value())
    {
        return true;
    }
    auto &filter = currentFileFilter.value();
    if (filter.extensions.size() == 0 && filter.mimeTypes.size() == 0)
    {
        return true;
    }
    std::string extension = path.extension().string();
    if (filter.extensions.size() != 0)
    {
        for (const auto &item : filter.extensions)
        {
            if (item == "*")
            {
                return true;
            }

            if (item == extension)
            {
                return true;
            }
        }
    }
    if (filter.mimeTypes.size() != 0)
    {
        const auto &mimeType = MimeTypes::MimeTypeFromExtension(extension);
        for (const auto &item : filter.mimeTypes)
        {
            if (item == "*")
            {
                return true;
            }
            if (item.ends_with("*"))
            {
                if (strncmp(mimeType.c_str(), item.c_str(), item.length() - 1) == 0)
                {
                    return true;
                }
            }
            else
            {
                if (mimeType == item)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void Lv2FileDialog::SetNoFilesLabel(const std::string &message)
{
    this->noFilesLabel->Text(message);
    auto visibility =
        message.length() == 0
            ? LvtkVisibility::Collapsed
            : LvtkVisibility::Visible;

    if (this->noFilesLabel->Style().Visibility() != visibility)
    {
        this->noFilesLabel->Style()
            .Visibility(visibility);
        InvalidateLayout();
    }
}

void Lv2FileDialog::SetSearchStatusMessage(const std::string &message)
{
    if (currentSearchStatusMessage != message)
    {
        currentSearchStatusMessage = message;
        searchMessageElement->Text(message);

        LvtkVisibility visibility = message.length() != 0 ? LvtkVisibility::Visible : LvtkVisibility::Collapsed;
        if (searchMessageElement->Style().Visibility() != visibility)
        {
            searchMessageElement->Style().Visibility(visibility);
            InvalidateLayout();
        }
    }
}

void Lv2FileDialog::OnSearchBarAnimate(double value)
{
    searchBarAnimationValue = value;

    if (value == 1)
    {
        searchBarSizer->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .MarginLeft(0) // to default
            .Width(0)
            .Visibility(LvtkVisibility::Visible);
        breadcrumbBar->Style()
            .Visibility(LvtkVisibility::Collapsed);
        searchButton->Style()
            .Visibility(LvtkVisibility::Collapsed);
    }
    else if (value == 0)
    {
        searchBarSizer->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .MarginLeft(0) // to default
            .Width(0)
            .Visibility(LvtkVisibility::Collapsed);
        breadcrumbBar->Style()
            .Visibility(LvtkVisibility::Visible);
        searchButton->Style()
            .Visibility(LvtkVisibility::Visible);
    }
    else
    {
        // exit if we don't have good layout data to do the animation with.
        if (searchButtonWidth == 0)
            return;

        searchButton->Style().Visibility(LvtkVisibility::Collapsed);
        searchBarSizer->Style()
            .Visibility(LvtkVisibility::Visible)
            .HorizontalAlignment(LvtkAlignment::Start);

        double minWidth = searchButtonWidth;
        double maxGapWidth = searchButtonWidth + breadcrumbBarWidth - 17; // maximum width we can sustain while showing breadcrumb bar.
        double maxWidth = searchButtonWidth + breadcrumbBarWidth;

        double width = minWidth + (maxWidth - minWidth) * (value);

        if (width < maxGapWidth)
        {

            breadcrumbBar->Style().Visibility(LvtkVisibility::Visible);
            searchBarSizer->Style()
                .Width(width)
                .MarginLeft(0); // to default
        }
        else
        {
            breadcrumbBar->Style().Visibility(LvtkVisibility::Collapsed);
            searchBarSizer->Style()
                .MarginLeft(maxWidth - width)
                .Width(width);
        }
    }
    searchBarSizer->InvalidateParentLayout();
    InvalidateLayout();
}

void Lv2FileDialog::OnLayoutComplete()
{

    // update the animation data in the case that a full
    // layout occurs while animating.
    if (searchBarAnimationValue == 0.0)
    {
        breadcrumbBarWidth = this->breadcrumbBar->ScreenBounds().Width();
        searchButtonWidth = this->searchButton->ScreenBounds().Width();
    }
    else if (searchBarAnimationValue == 1.0)
    {
        breadcrumbBarWidth = this->searchBarSizer->ScreenBounds().Width() - searchButtonWidth;
    }
    else
    {
        LvtkRectangle bounds;
        if (breadcrumbBar->Style().Visibility() != LvtkVisibility::Collapsed)
        {
            bounds = this->breadcrumbBar->ScreenBounds().Union(this->searchBarSizer->ScreenBounds());
        }
        else
        {
            bounds = this->searchBarSizer->ScreenBounds();
        }
        breadcrumbBarWidth = bounds.Width() - searchButtonWidth;
    }
}

void Lv2FileDialog::OnFileTypesChanged(const std::vector<Lv2FileFilter> &value)
{
    std::vector<LvtkDropdownItem> items;
    int64_t index = 0;
    for (const auto &fileType : value)
    {
        items.push_back(LvtkDropdownItem(index++, fileType.label));
    }
    this->FileTypeDropdownItems(items);
    OnFilterChanged();
}
void Lv2FileDialog::OnSelectedFileTypeChanged(int64_t value)
{
    OnFilterChanged();
}

void Lv2FileDialog::OnFilterChanged()
{
    if (SelectedFileType() >= 0 && SelectedFileType() < (int64_t)FileTypes().size())
    {
        currentFileFilter = FileTypes()[SelectedFileType()];
    }
    else
    {
        currentFileFilter.reset();
    }
    if (searchBoxOpen)
    {
        CancelSearchTimer();
        StartSearchTask();
    }
    else
    {
        FilesScrollOffset(0);
        SelectedFile("");
        LoadFileList();
    }
}

bool Lv2FileFilter::operator==(const Lv2FileFilter &other) const
{
    return label == other.label && extensions == other.extensions && mimeTypes == other.mimeTypes;
}

const std::string &Lv2FileDialog::DefaultDirectory() const { return defaultDirectory; }

Lv2FileDialog &Lv2FileDialog::DefaultDirectory(const std::string &path)
{
    this->defaultDirectory = path;
    return *this;
}

void Lv2FileDialog::AddPanel(size_t position, const LvtkFilePanel&panel_)
{
    FilePanel panel;
    panel.icon = panel_.icon;
    panel.label = panel_.label;
    panel.path = panel_.path;
    panel.locationType = LocationType::Path;
    this->panels.insert(this->panels.begin()+position,panel);
}
