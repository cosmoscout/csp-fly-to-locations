/* global IApi, CosmoScout */

(() => {
  /**
   * FlyTo Api
   */
  class FlyToLocationsApi extends IApi {
    /**
     * @inheritDoc
     * @type {string}
     */
    name = 'flyToLocations';

    /**
     * Adds a bookmark button to the grid of icon bookmarks.
     *
     * @param bookmarkID {number}
     * @param bookmarkJSON {string}
     */
    addGridBookmark(bookmarkID, bookmarkName, bookmarkIcon) {
      let button = CosmoScout.gui.loadTemplateContent('flytolocations-bookmarks-grid-button');

      button.innerHTML = button.innerHTML.replace(/%NAME%/g, bookmarkName).replace(/%ICON%/g, bookmarkIcon).trim();
      button.onclick = () => {
        CosmoScout.callbacks.bookmark.gotoLocation(bookmarkID);
      };

      document.getElementById('flytolocations-bookmarks-grid').appendChild(button);
    }

    /**
     * csp-fly-to-locations
     *
     * @param group {string}
     * @param text {string}
     */
    addListBookmark(bookmarkID, bookmarkName, bookmarkHasTime) {
      let listItem = CosmoScout.gui.loadTemplateContent('flytolocations-bookmarks-list-item');

      listItem.innerHTML = listItem.innerHTML.replace(/%TEXT%/g, bookmarkName).trim();

      document.getElementById('flytolocations-bookmarks-list').appendChild(listItem);

      CosmoScout.gui.initTooltips();
    }

    /**
     * csp-fly-to-locations
     *
     * @param group {string}
     * @param text {string}
     */
    removeBookmark(bookmarkID) {

    }
  }

  CosmoScout.init(FlyToLocationsApi);
})();
