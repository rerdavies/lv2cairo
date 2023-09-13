// Copyright (c) 2023 Robin Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
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

#pragma once

#include "lvtk/LvtkDialog.hpp"
#include "lvtk/LvtkBindingProperty.hpp"
#include <memory>
#include <filesystem>
#include <unordered_set>
#include "lvtk/LvtkBindingProperty.hpp"
#include "lvtk/LvtkAnimator.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/IcuString.hpp"


// forard declarations.
namespace lvtk
{
    class LvtkWindow;
    class LvtkButtonElement;
    class LvtkFlexGridElement;
    class LvtkTypographyElement;
    class LvtkEditBoxElement;
}
namespace lvtk::ui
{

    struct LvtkFileFilter {
        std::string label;
        std::vector<std::string> extensions;
        std::vector<std::string> mimeTypes;

        bool operator==(const LvtkFileFilter&other) const;
    };

    class Lv2FileDialog : public lvtk::LvtkDialog
    {
    public:
        using self = Lv2FileDialog;
        using super = lvtk::LvtkDialog;
        using ptr = std::shared_ptr<self>;
        static ptr Create(const std::string &title, const std::string &settingsKey) { return std::make_shared<self>(title, settingsKey); }

        Lv2FileDialog(const std::string &title, const std::string &settingsKey);

        virtual void Show(lvtk::LvtkWindow *parent) override;

        LvtkEvent<std::string> OK;

        BINDING_PROPERTY(SelectedLocation, int64_t, 2);

        BINDING_PROPERTY_REF(SelectedFile, std::string, std::string());

        BINDING_PROPERTY_REF(FileTypes, std::vector<LvtkFileFilter>, std::vector<LvtkFileFilter>())
        BINDING_PROPERTY(SelectedFileType, int64_t, 0)

    protected:
        BINDING_PROPERTY(OkEnabled, bool, false);
        BINDING_PROPERTY(BackButtonEnabled, bool, false);
        BINDING_PROPERTY(ForwardButtonEnabled, bool, false);
        BINDING_PROPERTY(FavoriteButtonEnabled, bool, false);

        BINDING_PROPERTY_REF(FavoriteIconSource, std::string, "FileDialog/favorite_unchecked.svg")
        void OnFileTypesChanged(const std::vector<LvtkFileFilter>& value);
        void OnSelectedFileTypeChanged(int64_t value);
    protected:
        BINDING_PROPERTY(SearchProgressActive,bool,false)

        virtual void OnLayoutComplete() override;
        virtual bool OnKeyDown(LvtkKeyboardEventArgs &event) override;
        virtual void OnClosing() override;
        virtual void OnSelectedFileChanged(const std::string &fileName);
        virtual void OnToggleFavorite();

        virtual LvtkElement::ptr Render() override;

        void OnCancel();
        void OnOk();

    private:
        IcuString::Ptr icuString; // strictly lifetime managment.

        BINDING_PROPERTY_REF(FileTypeDropdownItems,std::vector<LvtkDropdownItem>,std::vector<LvtkDropdownItem>())
        class SearchTask;

        LvtkAnimator searchBarAnimator;
        void OnSearchBarAnimate(double value);
        void UpdateSearchAnimationForLayout();
        void OnFilterChanged();

        std::shared_ptr<SearchTask> searchTask;

        enum class SearchStatus
        {
            Idle,
            Interrim,
            Complete
        };
        animation_clock_t::time_point searchStartTime;
        
        SearchStatus currentSearchStatus = SearchStatus::Complete;
        std::vector<std::string> currentSearchResults;
        void LoadSearchResults();
        void LoadSearchResults(const std::vector<std::string> &files, SearchStatus);

        AnimationHandle searchTimerHandle;
        observer_handle_t searchTextChangedHandle;
        EventHandle searchCancelHandle;
        void StartSearchTimer(bool noDelay);
        void CancelSearchTimer();

        void StartSearchTask();

        bool searchVisible = false;

        Lv2FileDialog &SearchVisible(bool visible);
        bool SearchVisible() const;
        enum class LocationType
        {
            None,
            Recent,
            Favorites,
            Path,
        };
        struct FilePanel
        {
            std::string label;
            std::string icon;
            std::string path;
            LocationType locationType = LocationType::None;

            bool operator==(const FilePanel &other) const;
        };

        struct FileLocation
        {
            std::string path;
            LocationType locationType = LocationType::None;

            bool operator==(const FileLocation &other) const;
        };

        static std::vector<FilePanel> gPanels;

        struct LvtkDialogFile;

        FilePanel currentPanel;
        FileLocation currentLocation;

        bool searchBoxOpen = false;
        FileLocation searchSavedLocation;

        BINDING_PROPERTY(FilesScrollOffset, double, 0)
        void SelectPanel(const FileLocation &location);

        void OnNavigateBack();
        void OnNavigateForward();
        void Navigate(const FileLocation &location);
        void Navigate(const std::filesystem::path &path);

        void OnOpenLocation(int64_t location);
        void OnFileSelected(const std::filesystem::path &path);

        void OnSelectedLocationChanged(int64_t location);

        bool IsFavorite(const std::string &fileName);

        std::string GetIcon(const LvtkDialogFile &file);
        std::string GetIcon(const std::filesystem::path &path);

        void LoadBreadcrumbBar();
        void LoadFiles(const std::filesystem::path &path);

        void LoadMixedDirectoryFiles(const std::vector<std::string> &files);
        void LoadFavorites();
        void LoadRecent();
        void LoadFileList();

        LvtkElement::ptr RenderPanel(size_t index, const FilePanel &location);
        LvtkElement::ptr RenderFileList();
        LvtkElement::ptr RenderBreadcrumb(const std::string &icon, const std::string &label, const std::string &path);
        LvtkElement::ptr RenderBreadcrumbBar();
        LvtkElement::ptr RenderSearchProgressBar();
        LvtkElement::ptr RenderSearchBar();
        LvtkElement::ptr RenderTopPanel();
        LvtkElement::ptr RenderLocations();
        LvtkElement::ptr RenderSearchMessage();
        LvtkElement::ptr RenderFilePanel();
        LvtkElement::ptr RenderFooter();
        LvtkElement::ptr RenderBody();

        LvtkElement::ptr searchButton;
        observer_handle_t searchButtonHandle;
        
        std::shared_ptr<LvtkButtonElement> okButton, cancelButton;
        EventHandle okEventHandle, cancelEventHandle;

        std::vector<LvtkElement::ptr> locations;
        std::shared_ptr<LvtkContainerElement> fileListContainer;
        std::unordered_set<std::string> favorites;

        std::vector<std::string> recentEntries;
        void SaveSettings();
        FileLocation LoadSettings();
        void TrimNonexistentFiles();

        void AddCurrentLocationToBackList();
        void AddCurrentLocationToForwardList();
        void ClearForwardList();
        void ClearBackList();


        std::vector<std::string> GetFavoritesVector();
        void OpenSearchBox();
        void CloseSearchBox(bool refreshContent = true);
        void DirectSearch();

        std::vector<FileLocation> navigateBackList;
        std::vector<FileLocation> navigateForwardList;

        std::shared_ptr<LvtkTypographyElement> noFilesLabel;
        std::shared_ptr<LvtkFlexGridElement> breadcrumbBar;
        std::shared_ptr<LvtkElement> searchBar;
        std::shared_ptr<LvtkElement> searchBarSizer;
        std::shared_ptr<LvtkEditBoxElement> searchEdit;
        std::shared_ptr<LvtkTypographyElement> searchMessageElement;


        std::string currentSearchStatusMessage;

        void SetSearchStatusMessage(const std::string &message);
        void SetNoFilesLabel(const std::string &message);

        struct Breadcrumb
        {
            std::string label;
            std::string path;
        };

        std::vector<Breadcrumb> GetBreadcrumbs(FilePanel &panel, const std::filesystem::path &path);

        std::mutex searchResultMutex;
        AnimationHandle searchResultHandle;
        double searchBarAnimationValue = 0;

        double breadcrumbBarWidth = 0;
        double searchButtonWidth = 0;
        bool FileTypeMatch(const std::filesystem::path &path) const;

        std::optional<LvtkFileFilter> currentFileFilter;
    };
}