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
  std::cout << "Loading: CosmoScout VR Plugin Fly to Locations" << std::endl;

  mPluginSettings = mAllSettings->mPlugins.at("csp-fly-to-locations");

  mGuiManager->addPluginTabToSideBarFromHTML(
      "Navigation", "location_on", "../share/resources/gui/fly-to-locations-tab.html");

  mGuiManager->addScriptToSideBarFromJS("../share/resources/gui/js/fly-to-locations-tab.js");

  for (auto const& settings : mPluginSettings.mTargets) {
    auto anchor = mAllSettings->mAnchors.find(settings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + settings.first + "\" defined in the settings.");
    }

    mGuiManager->getSideBar()->callJavascript(
        "add_celestial_body", anchor->second.mCenter, settings.second.mIcon);
  }

  mSolarSystem->pActiveBody.onChange().connect(
      [this](std::shared_ptr<cs::scene::CelestialBody> const& body) {
        mGuiManager->getHeaderBar()->callJavascript(
            "set_active_planet", body->getCenterName(), body->getFrameName());

        mGuiManager->getSideBar()->callJavascript("clear_container", "location-tabs-area");

        auto const& planet =
            mPluginSettings.mTargets.find(mSolarSystem->pActiveBody.get()->getCenterName());

        if (planet != mPluginSettings.mTargets.end()) {
          auto const& locations = planet->second.mLocations;

          for (auto loc : locations) {
            mGuiManager->getSideBar()->callJavascript("add_location", loc.second.mGroup, loc.first);
          }
        }
      });

  mGuiManager->getSideBar()->registerCallback<std::string>(
      "fly_to", [this](std::string const& name) {
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
      });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  mGuiManager->getSideBar()->unregisterCallback("fly_to");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::flytolocations
