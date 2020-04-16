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
#include "logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::flytolocations::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase; // NOLINT(cppcoreguidelines-owning-memory)
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::flytolocations {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Location& o) {
  o.mLatitude  = cs::core::parseProperty<double>("latitude", j);
  o.mLongitude = cs::core::parseProperty<double>("longitude", j);
  o.mExtent    = cs::core::parseProperty<double>("extent", j);
  o.mGroup     = cs::core::parseProperty<std::string>("group", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Target& o) {
  o.mIcon = cs::core::parseProperty<std::string>("icon", j);

  auto iter = j.find("locations");
  if (iter != j.end()) {
    o.mLocations = cs::core::parseMap<std::string, Plugin::Settings::Location>("locations", j);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  cs::core::parseSection("csp-atmospheres", [&] {
    o.mTargets = cs::core::parseMap<std::string, Plugin::Settings::Target>("targets", j);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {

  logger()->info("Loading plugin...");

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

  mActiveBodyConnection = mSolarSystem->pActiveBody.connectAndTouch(
      [this](std::shared_ptr<cs::scene::CelestialBody> const& body) {
        mGuiManager->getGui()->callJavascript("CosmoScout.gui.clearHtml", "location-tabs-area");

        if (body) {
          auto const& planet = mPluginSettings.mTargets.find(body->getCenterName());

          if (planet != mPluginSettings.mTargets.end()) {
            auto const& locations = planet->second.mLocations;

            for (auto const& loc : locations) {
              mGuiManager->getGui()->callJavascript(
                  "CosmoScout.flyToLocations.addLocation", loc.second.mGroup, loc.first);
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
          if (anchor->second.mCenter == mSolarSystem->pActiveBody.get()->getCenterName()) {
            auto const& location = planet.second.mLocations.find(name);
            if (location != planet.second.mLocations.end()) {
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

  logger()->info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  logger()->info("Unloading plugin...");

  mGuiManager->removePluginTab("Navigation");

  mGuiManager->getGui()->unregisterCallback("flyToLocations.flyTo");
  mGuiManager->getGui()->callJavascript("CosmoScout.gui.unregisterHtml", "fly-to-locations");
  mGuiManager->getGui()->callJavascript(
      "CosmoScout.gui.unregisterCss", "css/csp-fly-to-locations.css");

  mSolarSystem->pActiveBody.disconnect(mActiveBodyConnection);

  logger()->info("Unloading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::flytolocations
