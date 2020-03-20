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

void from_json(nlohmann::json const& j, Plugin::Settings::Location& o) {
  cs::core::Settings::deserialize(j, "latitude", o.mLatitude);
  cs::core::Settings::deserialize(j, "longitude", o.mLongitude);
  cs::core::Settings::deserialize(j, "extent", o.mExtent);
  cs::core::Settings::deserialize(j, "group", o.mGroup);
}

void to_json(nlohmann::json& j, Plugin::Settings::Location const& o) {
  cs::core::Settings::serialize(j, "latitude", o.mLatitude);
  cs::core::Settings::serialize(j, "longitude", o.mLongitude);
  cs::core::Settings::serialize(j, "extent", o.mExtent);
  cs::core::Settings::serialize(j, "group", o.mGroup);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings::Target& o) {
  cs::core::Settings::deserialize(j, "icon", o.mIcon);
  cs::core::Settings::deserialize(j, "locations", o.mLocations);
}

void to_json(nlohmann::json& j, Plugin::Settings::Target const& o) {
  cs::core::Settings::serialize(j, "icon", o.mIcon);
  cs::core::Settings::serialize(j, "locations", o.mLocations);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings& o) {
  cs::core::Settings::deserialize(j, "targets", o.mTargets);
}

void to_json(nlohmann::json& j, Plugin::Settings const& o) {
  cs::core::Settings::serialize(j, "targets", o.mTargets);
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

  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/js/csp-fly-to-locations.js");
  mGuiManager->addCssToGui("css/csp-fly-to-locations.css");

  mGuiManager->addPluginTabToSideBarFromHTML(
      "Navigation", "location_on", "../share/resources/gui/fly-to-locations-tab.html");

  for (auto const& settings : mPluginSettings.mTargets) {
    auto anchor = mAllSettings->mAnchors.find(settings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + settings.first + "\" defined in the settings.");
    }

    mGuiManager->getGui()->callJavascript("CosmoScout.flyToLocations.addCelestialBody",
        anchor->second.mCenter, settings.second.mIcon);
  }

  mActiveBodyConnection = mSolarSystem->pActiveBody.onChange().connect(
      [this](std::shared_ptr<cs::scene::CelestialBody> const& body) {
        mGuiManager->getGui()->callJavascript("CosmoScout.gui.clearHtml", "location-tabs-area");

        if (body) {
          auto const& planet = mPluginSettings.mTargets.find(body->getCenterName());

          if (planet != mPluginSettings.mTargets.end()) {
            auto const& locations = planet->second.mLocations;

            if (locations) {
              for (auto loc : locations.value()) {
                mGuiManager->getGui()->callJavascript(
                    "CosmoScout.flyToLocations.addLocation", loc.second.mGroup, loc.first);
              }
            }
          }
        }
      });

  mGuiManager->getGui()->registerCallback("flyToLocations.flyTo",
      "Fly the observer to the given bookmark.", std::function([this](std::string&& name) {
        if (!mSolarSystem->pActiveBody.get()) {
          return;
        }

        for (auto const& planet : mPluginSettings.mTargets) {
          auto anchor = mAllSettings->mAnchors.find(planet.first);
          if (anchor->second.mCenter == mSolarSystem->pActiveBody.get()->getCenterName() &&
              planet.second.mLocations) {
            auto const& location = planet.second.mLocations.value().find(name);
            if (location != planet.second.mLocations.value().end()) {
              glm::dvec2 lngLat(location->second.mLongitude, location->second.mLatitude);
              lngLat        = cs::utils::convert::toRadians(lngLat);
              double height = mSolarSystem->pActiveBody.get()->getHeight(lngLat);
              mSolarSystem->flyObserverTo(mSolarSystem->pActiveBody.get()->getCenterName(),
                  mSolarSystem->pActiveBody.get()->getFrameName(), lngLat,
                  location->second.mExtent + height, 5.0);
              mGuiManager->showNotification("Travelling", "to " + location->first, "send");
            }
          }
        }
      }));

  spdlog::info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  spdlog::info("Unloading plugin...");

  mGuiManager->getGui()->unregisterCallback("flyToLocations.flyTo");
  mSolarSystem->pActiveBody.onChange().disconnect(mActiveBodyConnection);
  mGuiManager->getGui()->callJavascript("CosmoScout.gui.unregisterHtml", "fly-to-locations");
  mGuiManager->getGui()->callJavascript(
      "CosmoScout.gui.unregisterCss", "css/csp-fly-to-locations.css");

  spdlog::info("Unloading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::flytolocations
