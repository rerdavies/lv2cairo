// Copyright (c) 2023 Robin E. R. Davies
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

#include "lv2c/Lv2cDialog.hpp"
#include "lv2c/Lv2cBindingProperty.hpp"
#include <memory>
#include <filesystem>
#include <unordered_set>
#include "lv2c/Lv2cBindingProperty.hpp"
#include "lv2c/Lv2cAnimator.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/IcuString.hpp"


// forard declarations.
namespace lvtk
{
    class Lv2cWindow;
    class Lv2cButtonElement;
    class Lv2cFlexGridElement;
    class Lv2cTypographyElement;
    class Lv2cEditBoxElement;
}
namespace lvtk::ui
{

    struct Lv2FileFilter {
        std::string label;
        std::vector<std::string> extensions;
        std::vector<std::string> mimeTypes;

        bool operator==(const Lv2FileFilter&other) const;
    };

    class Lv2FileDialog : public lvtk::Lv2cDialog
    {
    public:
        using self = Lv2FileDialog;
        using super = lvtk::Lv2cDialog;
        using ptr = std::shared_ptr<self>;
        static ptr Create(const std::string &title, const std::string &settingsKey) { return std::make_shared<self>(title, settingsKey); }

        Lv2FileDialog(const std::string &title, const std::string &settingsKey);

        virtual void Show(lvtk::Lv2cWindow *parent) override;

        Lv2cEvent<std::string> OK;

        Lv2cEvent<void> Cancelled;

        BINDING_PROPERTY(SelectedLocation, int64_t, 2);

        BINDING_PROPERTY_REF(SelectedFile, std::string, std::string());

        BINDING_PROPERTY_REF(FileTypes, std::vector<Lv2FileFilter>, std::vector<Lv2FileFilter>())
        BINDING_PROPERTY(SelectedFileType, int64_t, 0)

        const std::string& DefaultDirectory() const;
        Lv2FileDialog&DefaultDirectory(const std::string&path);

        struct Lv2cFilePanel
        {
            std::string label;
            std::string icon;
            std::string path;
        };

        void AddPanel(size_t position, const Lv2cFilePanel&panel);

        /// Show the "Clear Value" button in the dialog UI.
        BINDING_PROPERTY(ShowClearValue, bool, false);

    protected:
        BINDING_PROPERTY(OkEnabled, bool, false);
        BINDING_PROPERTY(BackButtonEnabled, bool, false);
        BINDING_PROPERTY(ForwardButtonEnabled, bool, false);
        BINDING_PROPERTY(FavoriteButtonEnabled, bool, false);

        BINDING_PROPERTY_REF(FavoriteIconSource, std::string, "FileDialog/favorite_unchecked.svg")
        void OnFileTypesChanged(const std::vector<Lv2FileFilter>& value);
        void OnSelectedFileTypeChanged(int64_t value);
    protected:
        BINDING_PROPERTY(SearchProgressActive,bool,false)

        virtual void OnLayoutComplete() override;
        virtual bool OnKeyDown(Lv2cKeyboardEventArgs &event) override;
        virtual void OnClosing() override;
        virtual void OnSelectedFileChanged(const std::string &fileName);
        virtual void OnToggleFavorite();

        virtual Lv2cElement::ptr Render() override;

        void OnCancel();
        void OnOk();
        void OnClearValue();

    private:
        using clock_t = std::chrono::steady_clock;

        std::string defaultDirectory;
        bool okClose = false;
        IcuString::Ptr icuString; // strictly lifetime managment.

        BINDING_PROPERTY_REF(FileTypeDropdownItems,std::vector<Lv2cDropdownItem>,std::vector<Lv2cDropdownItem>())
        class SearchTask;

        Lv2cAnimator searchBarAnimator;
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

        animation_clock_t::time_point lastFileClickTime;
        Lv2cPoint lastFileClickPoint { -1000,-1000};

        
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
        std::vector<FilePanel> panels;

        struct Lv2cDialogFile;

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
        void OnFileSelected(const std::filesystem::path &path, const Lv2cMouseEventArgs&mouseEventArgs);

        void OnSelectedLocationChanged(int64_t location);

        bool IsFavorite(const std::string &fileName);

        std::string GetIcon(const Lv2cDialogFile &file);
        std::string GetIcon(const std::filesystem::path &path);

        void LoadBreadcrumbBar();
        void LoadFiles(const std::filesystem::path &path);

        void LoadMixedDirectoryFiles(const std::vector<std::string> &files);
        void LoadFavorites();
        void LoadRecent();
        void LoadFileList();

        Lv2cElement::ptr RenderPanel(size_t index, const FilePanel &location);
        Lv2cElement::ptr RenderFileList();
        Lv2cElement::ptr RenderBreadcrumb(const std::string &icon, const std::string &label, const std::string &path);
        Lv2cElement::ptr RenderBreadcrumbBar();
        Lv2cElement::ptr RenderSearchProgressBar();
        Lv2cElement::ptr RenderSearchBar();
        Lv2cElement::ptr RenderTopPanel();
        Lv2cElement::ptr RenderLocations();
        Lv2cElement::ptr RenderSearchMessage();
        Lv2cElement::ptr RenderFilePanel();
        Lv2cElement::ptr RenderFooter();
        Lv2cElement::ptr RenderBody();

        Lv2cElement::ptr searchButton;
        observer_handle_t searchButtonHandle;
        
        std::shared_ptr<Lv2cButtonElement> okButton, cancelButton;
        EventHandle okEventHandle, cancelEventHandle, clearValueEventHandle;

        std::vector<Lv2cElement::ptr> locations;
        std::shared_ptr<Lv2cContainerElement> fileListContainer;
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

        std::shared_ptr<Lv2cTypographyElement> noFilesLabel;
        std::shared_ptr<Lv2cFlexGridElement> breadcrumbBar;
        std::shared_ptr<Lv2cElement> searchBar;
        std::shared_ptr<Lv2cElement> searchBarSizer;
        std::shared_ptr<Lv2cEditBoxElement> searchEdit;
        std::shared_ptr<Lv2cTypographyElement> searchMessageElement;


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

        std::optional<Lv2FileFilter> currentFileFilter;
    };
}