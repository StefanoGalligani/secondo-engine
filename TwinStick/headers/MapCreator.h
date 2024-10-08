#pragma once

#include <sgEngine.h>
#include <glm/glm/gtc/random.hpp>

class MapCreator {
private:
    sg::Renderer* _renderer;
    sg::Model* _treeModel;
    sg::Model* _lampModel;

    sg::Object3D* _mapObj;
    sg::Object3D* _shedObj;
    sg::Object3D* _siloObj;
    sg::Object3D* _siloObj2;

    std::vector<sg::Object3D*> _lampObjs;
    std::vector<sg::PointLight3D*> _lampLights;
    bool _topLightsInScene;

    std::vector<sg::Object3D*> _trees;

    void GenerateTrees(int n, glm::vec3 min, glm::vec3 max) {
        for (int i = 0; i < n; i++) {
            sg::Object3D* treeObj = new sg::Object3D();
            treeObj->SetModel(_treeModel);
            treeObj->Lit = true;
            treeObj->CastsShadows = true;
            treeObj->ReceivesShadows = true;
            treeObj->SetGlobalPosition(glm::linearRand(min, max));
            treeObj->RotateGlobal(treeObj->GlobalUp(), glm::linearRand(0.0f, 3.1415926535f));
            _trees.push_back(treeObj);
            _renderer->AddObject(treeObj);
        }
    }

    void PlaceLamps() {
        _lampModel = new sg::Model();
        _lampModel->LoadFromObj("res/models/streetlamp.obj");
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                sg::Object3D* lampObj = new sg::Object3D();
                lampObj->SetModel(_lampModel);
                lampObj->SetGlobalPosition(-27 + i * 54, 0, -18 + j * 54);
                lampObj->Lit = true;
                lampObj->CastsShadows = true;
                lampObj->ReceivesShadows = true;

                sg::PointLight3D* lampLight = new sg::PointLight3D(512, 0.05, 20);
                lampLight->SetIntensity(2);
                lampLight->SetColor(glm::vec3(1, 0.6f, 0));
                lampLight->SetLocalPosition(glm::vec3(0, 9.85f, 0));
                lampObj->AddChild(lampLight, true);

                _lampObjs.push_back(lampObj);
                _lampLights.push_back(lampLight);
                _renderer->AddObject(lampObj);
                if (j == 0) {
                    _renderer->AddLight(lampLight);
                }
            }
        }
        _topLightsInScene = true;
    }

public:
	MapCreator(sg::Renderer* renderer) {
        _renderer = renderer;
        _mapObj = new sg::Object3D();
        _shedObj = new sg::Object3D();
        _siloObj = new sg::Object3D();
        _siloObj2 = new sg::Object3D();
        _treeModel = new sg::Model();

        _mapObj->LoadModelFromObj("res/models/map.obj");
        _mapObj->Lit = true;
        _mapObj->ReceivesShadows = true;
        _mapObj->PerformFrustumCheck = false;

        _shedObj->LoadModelFromObj("res/models/shed.obj");
        _shedObj->Lit = true;
        _shedObj->CastsShadows = true;
        _shedObj->ReceivesShadows = true;

        _siloObj->LoadModelFromObj("res/models/silo.obj");
        _siloObj->Lit = true;
        _siloObj->CastsShadows = true;
        _siloObj->ReceivesShadows = true;
        _siloObj2->SetModel(_siloObj->GetModel());
        _siloObj2->Lit = true;
        _siloObj2->CastsShadows = true;
        _siloObj2->ReceivesShadows = true;
        _siloObj2->SetGlobalPosition(0, 0, 15);

        renderer->AddObject(_mapObj);
        renderer->AddObject(_shedObj);
        renderer->AddObject(_siloObj);
        renderer->AddObject(_siloObj2);

        _treeModel->LoadFromObj("res/models/tree.obj");
        GenerateTrees(10, glm::vec3(-40, 0, -30), glm::vec3(-13, 0, -23));
        GenerateTrees(10, glm::vec3(13, 0, -30), glm::vec3(40, 0, -23));
        GenerateTrees(10, glm::vec3(-40, 0, 15), glm::vec3(-32, 0, 40));
        GenerateTrees(15, glm::vec3(30, 0, -30), glm::vec3(40, 0, 40));
        GenerateTrees(15, glm::vec3(-25, 0, 40), glm::vec3(25, 0, 43));

        PlaceLamps();
	}

    void SwapLights() {
        if (_topLightsInScene) {
            _renderer->RemoveLight(_lampLights[0]);
            _renderer->RemoveLight(_lampLights[2]);
            _renderer->AddLight(_lampLights[1]);
            _renderer->AddLight(_lampLights[3]);
        } else {
            _renderer->RemoveLight(_lampLights[1]);
            _renderer->RemoveLight(_lampLights[3]);
            _renderer->AddLight(_lampLights[0]);
            _renderer->AddLight(_lampLights[2]);
        }
        _topLightsInScene = !_topLightsInScene;
    }

	~MapCreator() {
        delete(_mapObj);
        delete(_shedObj);
        delete(_siloObj);
        delete(_siloObj2);
        delete(_treeModel);
        delete(_lampModel);
        for (const auto& tree : _trees) {
            delete(tree);
        }
        for (const auto& lamp : _lampObjs) {
            delete(lamp);
        }
        for (const auto& light : _lampLights) {
            delete(light);
        }
	}
};