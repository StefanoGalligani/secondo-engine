#include <sgEngine.h>
#include <sstream>
#include <Player.h>
#include <EnemyManager.h>
#include <Bullet.h>
#include <MapCreator.h>
#include <irrKlang.h>
using namespace irrklang;

sg::Renderer* renderer;
Player* player;
EnemyManager* enemyManager;
MapCreator* mapCreator;
sg::Model* bulletModel;
sg::DirectionalLight3D* sunLight;
sg::AmbientLight* ambientLight;
sg::PointLight3D* shootLight;
int shootLightPresent = 0;

bool showTriangulation = false;
bool minimized = false;

float resx = 1080;
float resy = 720;
int shadowResx = 1024;
int shadowResy = 1024;

ISoundEngine* SoundEngine = createIrrKlangDevice();

#define PLAYER_SPEED 10
#define ENEMY_SPEED 4
#define BULLET_SPEED 50
#define BULLET_LIFETIME 1

class sgGame {
public:
    void run() {
        if (!initWindow()) return;
        initGame();
        mainLoop();
        cleanup();
        closeApplication();
    }

private:
    bool initWindow() {
        if (!glfwInit())
            return -1;

        glfwWindowHint(GLFW_SAMPLES, 4);
        GLFWwindow* window = glfwCreateWindow(resx, resy, "TwinStick", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);

        if (glewInit() != GLEW_OK) {
            std::cout << "Error with Glew" << std::endl;
            return -1;
        }
        std::cout << glGetString(GL_VERSION) << std::endl;

        renderer = new sg::Renderer();
        if (renderer->InitRenderer(window, resx, resy) < 0) return false;
        return true;
    }

    void initGame() {
        BindInputs();
        InitObjects();
    }

    int averageFrameRate(int newFrameRate) {
        static std::vector<int> frameRates = std::vector<int>();
        static int sum = 0;
        sum += newFrameRate;
        frameRates.push_back(newFrameRate);
        if (frameRates.size() > 30) {
            sum -= frameRates[0];
            frameRates.erase(frameRates.begin());
        }
        return sum / frameRates.size();
    }

    void mainLoop() {
        printf("Starting rendering\n");
        float previousPlayerZ = player->GetGlobalPosition().z - 15;
        while (!renderer->Terminated())
        {
            if (!minimized) {
                if (checkGameOver()) {
                    cleanup();
                    initGame();
                }
                float newPlayerZ = player->GetGlobalPosition().z - 15;
                if (previousPlayerZ * newPlayerZ < 0) mapCreator->SwapLights();
                previousPlayerZ = newPlayerZ;

                int fps = renderer->RenderFrame();
                std::stringstream ss{};
                ss << "TwinStick [" << averageFrameRate(fps) << " FPS]";
                glfwSetWindowTitle(renderer->GetWindow(), ss.str().c_str());

                if (shootLightPresent > 0 && --shootLightPresent == 0) {
                    renderer->RemoveLight(shootLight);
                }
            }
            glfwPollEvents();
        }
    }

    bool checkGameOver() {
        return enemyManager->CheckCollision(player->GetGlobalPosition());
    }

    void cleanup() {
        printf("Terminating");
        delete(player);
        delete(enemyManager);
        delete(bulletModel);
        delete(mapCreator);
        delete(sunLight);
        delete(ambientLight);
        delete(shootLight);
        shootLightPresent = 0;
        renderer->RemoveAllEntities();
    }

    void closeApplication() {
        renderer->DestroyWindow();
        delete(renderer);
        glfwTerminate();
    }

    void InitObjects() {
        printf("Initializing objects\n");
        player = new Player(renderer, PLAYER_SPEED, shadowResx, shadowResy, resx, resy);
        enemyManager = new EnemyManager(renderer, ENEMY_SPEED, player, "res/models/zombie.obj");
        mapCreator = new MapCreator(renderer);
        bulletModel = new sg::Model();
        bulletModel->LoadFromObj("res/models/projectile.obj");

        sunLight = new sg::DirectionalLight3D(shadowResx*2, shadowResy*2, 35, 1, 50, 130, glm::vec3(0.1, -0.5, -0.5));
        sunLight->SetIntensity(0.2f);
        renderer->AddLight(sunLight);

        ambientLight = new sg::AmbientLight(0.12f);
        renderer->AddLight(ambientLight);

        shootLight = new sg::PointLight3D(128, 0.05, 50);
        shootLight->SetColor(glm::vec3(1, 0.2, 0.2));
        shootLight->SetIntensity(10);
    }

#pragma region input
    void BindInputs() {
        BindInput(sg::Key_Esc_Down, onEscKeyPressed);
        BindInput(sg::Key_Space_Down, onSpaceKeyPressed);
        BindInput(sg::Mouse_Left_Down, onLeftMouseButtonClick);
        BindInput(sg::Mouse_Position, onMouseDrag);
        BindInput(sg::Window_Resize, onWindowResize);

        BindInput(sg::Key_W_Down, onWPressed);
        BindInput(sg::Key_W_Up, onWReleased);
        BindInput(sg::Key_S_Down, onSPressed);
        BindInput(sg::Key_S_Up, onSReleased);
        BindInput(sg::Key_A_Down, onAPressed);
        BindInput(sg::Key_A_Up, onAReleased);
        BindInput(sg::Key_D_Down, onDPressed);
        BindInput(sg::Key_D_Up, onDReleased);
    }

    void BindInput(int cmd, sg::sgKeyOrMouseFun callback) {
        sg::InputManager::Instance()->BindInput(renderer->GetWindow(), cmd, callback);
    }
    void BindInput(int cmd, sg::sgCursorPosFun callback) {
        sg::InputManager::Instance()->BindInput(renderer->GetWindow(), cmd, callback);
    }
    void BindInput(int cmd, sg::sgWindowSizeFun callback) {
        sg::InputManager::Instance()->BindInput(renderer->GetWindow(), cmd, callback);
    }

    static void onEscKeyPressed(int mods) {
        renderer->DestroyWindow();
    }

    static void onSpaceKeyPressed(int mods) {
        showTriangulation = !showTriangulation;
        renderer->SetShowTriangulation(showTriangulation);
    }

    static void onWindowResize(int x, int y) {
        resx = x;
        resy = y;
        renderer->SetResolution(resx, resy);
        minimized = (x * y == 0);
        if (!minimized) player->UpdateCameraResolution(resx, resy);
    }

    static void onLeftMouseButtonClick(int mods) {
        Bullet* bullet = new Bullet(
            player->GetGlobalPosition() + player->GetDirection() * 1.4f,
            player->GetDirection(),
            BULLET_SPEED,
            BULLET_LIFETIME,
            enemyManager,
            renderer,
            bulletModel
        );
        renderer->AddObject(bullet);

        shootLight->SetGlobalPosition(player->GetGlobalPosition() + player->GetDirection() * 1.4f + glm::vec3(0, 1.4571, 0));
        if (shootLightPresent == 0) renderer->AddLight(shootLight);
        shootLightPresent = 1;

        SoundEngine->play2D("res/sfx/Gun.wav");
    }

    static void onWPressed(int mods) {
        player->SetVert(-1, true);
    }

    static void onWReleased(int mods) {
        player->SetVert(-1, false);
    }

    static void onSPressed(int mods) {
        player->SetVert(1, true);
    }

    static void onSReleased(int mods) {
        player->SetVert(1, false);
    }

    static void onAPressed(int mods) {
        player->SetHoriz(-1, true);
    }

    static void onAReleased(int mods) {
        player->SetHoriz(-1, false);
    }

    static void onDPressed(int mods) {
        player->SetHoriz(1, true);
    }

    static void onDReleased(int mods) {
        player->SetHoriz(1, false);
    }

    static void onMouseDrag(double xpos, double ypos) {
        player->SetNewDirection(xpos - resx/2, ypos - resy/2);
    }
#pragma endregion
};

int main(int argc, char* argv[]) {
    sgGame game;

    try {
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}