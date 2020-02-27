////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/GuiManager.hpp"
#include "../../../src/cs-core/InputManager.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/convert.hpp"
#include "../../../src/cs-utils/logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::flytolocations::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::flytolocations {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Bookmark& o) {
  o.mLatitude  = cs::core::parseProperty<double>("latitude", j);
  o.mLongitude = cs::core::parseProperty<double>("longitude", j);
  o.mHeight    = cs::core::parseProperty<double>("height", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Minimap& o) {
  o.mMapServer     = cs::core::parseProperty<std::string>("mapserver", j);
  o.mLayer         = cs::core::parseOptional<std::string>("layer", j);
  o.mAttribution   = cs::core::parseOptional<std::string>("attribution", j);
  o.mCircumference = cs::core::parseProperty<double>("circumference", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Target& o) {
  o.mIcon = cs::core::parseProperty<std::string>("icon", j);

  auto iter = j.find("bookmarks");
  if (iter != j.end()) {
    o.mBookmarks = cs::core::parseMap<std::string, Plugin::Settings::Bookmark>("bookmarks", j);
  }

  o.mMinimap = cs::core::parseOptional<Plugin::Settings::Minimap>("minimap", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  cs::core::parseSection("csp-atmospheres", [&] {
    o.mTargets = cs::core::parseMap<std::string, Plugin::Settings::Target>("targets", j);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin::Plugin() {
  // Create default logger for this plugin.
  spdlog::set_default_logger(cs::utils::logger::createLogger("csp-fly-to-locations"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {

  spdlog::info("Loading plugin...");

  mPluginSettings = mAllSettings->mPlugins.at("csp-fly-to-locations");

  mGuiManager->addHtmlToGui(
      "fly-to-locations", "../share/resources/gui/fly-to-locations-templates.html");

  mGuiManager->addPluginTabToSideBarFromHTML(
      "Navigation", "location_on", "../share/resources/gui/fly-to-locations-tab.html");

  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/leaflet/leaflet.js");
  mGuiManager->addCssToGui("third-party/leaflet/leaflet.css");

  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/js/csp-fly-to-locations.js");
  mGuiManager->addCssToGui("css/csp-fly-to-locations.css");


  for (auto const& settings : mPluginSettings.mTargets) {
    auto anchor = mAllSettings->mAnchors.find(settings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + settings.first + "\" defined in the settings.");
    }

    mGuiManager->getGui()->callJavascript(
        "CosmoScout.flyto.addCelestialBody", anchor->second.mCenter, settings.second.mIcon);
  }

  mActiveBodyConnection = mSolarSystem->pActiveBody.onChange().connect(
      [this](std::shared_ptr<cs::scene::CelestialBody> const& body) {
        mGuiManager->getGui()->callJavascript("CosmoScout.clearHtml", "location-tabs-area");
        bool enableMinimap = false;

        if (body) {
          auto const& planet = mPluginSettings.mTargets.find(body->getCenterName());

          if (planet != mPluginSettings.mTargets.end()) {

            if (planet->second.mMinimap) {
              enableMinimap = true;
              std::string layer = planet->second.mMinimap->mLayer ? *planet->second.mMinimap->mLayer : "";
              std::string attribution = planet->second.mMinimap->mAttribution ? *planet->second.mMinimap->mAttribution : "";
              mGuiManager->getGui()->callJavascript(
                  "CosmoScout.flyto.configureMinimap", planet->second.mMinimap->mMapServer, layer, attribution, planet->second.mMinimap->mCircumference);
            }

            auto const& bookmarks = planet->second.mBookmarks;

            mGuiManager->getGui()->callJavascript("CosmoScout.flyto.clearBookmarks");
            for (auto loc : bookmarks) {
              mGuiManager->getGui()->callJavascript(
                  "CosmoScout.flyto.addBookmark", loc.first, loc.second.mLatitude, loc.second.mLongitude);
            }
          }
        }
        
        mGuiManager->getGui()->callJavascript("CosmoScout.flyto.enableMinimap", enableMinimap);
        mGuiManager->getGui()->callJavascript("CosmoScout.flyto.setActivePlanet", body->getCenterName());
      });

  mGuiManager->getGui()->registerCallback<std::string>("fly_to", [this](std::string const& name) {
    if (!mSolarSystem->pActiveBody.get()) {
      return;
    }

    for (auto const& planet : mPluginSettings.mTargets) {
      auto anchor = mAllSettings->mAnchors.find(planet.first);
      if (anchor->second.mCenter == mSolarSystem->pActiveBody.get()->getCenterName()) {
        auto const& bookmark = planet.second.mBookmarks.find(name);
        if (bookmark != planet.second.mBookmarks.end()) {
          glm::dvec2 lngLat(bookmark->second.mLongitude, bookmark->second.mLatitude);
          lngLat        = cs::utils::convert::toRadians(lngLat);
          double height = mSolarSystem->pActiveBody.get()->getHeight(lngLat);
          mSolarSystem->flyObserverTo(mSolarSystem->pActiveBody.get()->getCenterName(),
              mSolarSystem->pActiveBody.get()->getFrameName(), lngLat,
              bookmark->second.mHeight + height, 5.0);
          mGuiManager->showNotification("Travelling", "to " + bookmark->first, "send");
        }
      }
    }
  });

  spdlog::info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  spdlog::info("Unloading plugin...");

  mGuiManager->getGui()->unregisterCallback("fly_to");
  mSolarSystem->pActiveBody.onChange().disconnect(mActiveBodyConnection);
  mGuiManager->getGui()->callJavascript("CosmoScout.unregisterHtml", "fly-to-locations");
  mGuiManager->getGui()->callJavascript("CosmoScout.unregisterCss", "css/csp-fly-to-locations.css");

  spdlog::info("Unloading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::flytolocations
