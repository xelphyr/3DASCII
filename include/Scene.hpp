#pragma once

#include "SceneObject.hpp"
#include "Light.hpp"
#include "camera.hpp"
#include "SceneSettings.hpp"

class Scene
{
    public:
        Scene();
        ~Scene();

        std::vector<SceneObject>& GetSceneObjects();
        std::vector<SceneObject>& GetAssetObjects();
        void AddObjectToScene(std::string id);
        void AddObjectToScene(SceneObject object);
        void AddObjectToAssets(SceneObject object);

        bool RemoveObjectFromScene(const std::string& id);
        bool RemoveObjectFromAssets(const std::string& id);

        void GetObjectList() const;
        void GetObjectData();

        bool SaveData(std::vector<Light>& lights, Camera& camera, SceneSettings& settings);
        bool GetData(std::vector<Light>& lights, Camera& camera, SceneSettings& settings);
    private:
        std::vector<SceneObject> m_assetObjects; //Objects that can be placed multiple times into the scene.
        std::vector<SceneObject> m_sceneObjects; //Objects present within the scene.
};
