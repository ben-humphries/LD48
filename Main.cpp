#include "Dawn/Dawn.h"

#include <time.h>

#define PLAYER_OFFSET 2.5

float distance(Dawn::Vec2 v1, Dawn::Vec2 v2) {
	return sqrt((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
}

float randRange(float low, float high) {
	return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low)));
}

class BubbleColumn {
public:
	BubbleColumn(Dawn::Scene& scene, Dawn::Texture * tex) : m_scene(scene) {
		e = m_scene.addEntity();
		m_scene.addComponent<Dawn::SpriteRendererComponent>(e);
		auto& spriteRenderer = m_scene.getComponent<Dawn::SpriteRendererComponent>(e);
		spriteRenderer.texture = tex;
		//spriteRenderer.color = Dawn::Vec4(1.0, 0.0, 0.0, 1.0);
	}

	~BubbleColumn() {
		m_scene.deleteEntity(e);
	}

	void setPosition(Dawn::Vec3 position) {
		auto& transform = m_scene.getComponent<Dawn::TransformComponent>(e);
		transform.position = position;
	}

	const Dawn::Vec3 getPosition() {
		auto& transform = m_scene.getComponent<Dawn::TransformComponent>(e);
		return transform.position;
	}

	Dawn::Entity e;
private:
	Dawn::Scene& m_scene;

};

class DawnApp : public Dawn::Application {
public:

	Dawn::Scene scene = Dawn::Scene();

	Dawn::Texture scuba;
	Dawn::Texture bubbleColumn;
	Dawn::Texture bg;
	Dawn::Texture bubble;

	Dawn::Entity scubaEntity;

	//Main Camera entity will always = 1 in Dawn. Hacky but works for now
	Dawn::Entity cameraEntity = 1;

	Dawn::Vec3 acceleration = Dawn::Vec3();
	Dawn::Vec3 velocity = Dawn::Vec3();
	Dawn::Vec3 constVelocity = Dawn::Vec3(0, -2, 0);
	float angularVelocity = 0.0f;

	Dawn::Vec3 cameraVelocity = Dawn::Vec3();

	std::vector<BubbleColumn *> bubbleColumns;

	Dawn::Entity background;
	Dawn::Entity background2;
	bool backgroundBit = true;

	Dawn::Entity bubbleEntity;

	Dawn::Entity oxygenEntity;
	float oxygen = 1;
	Dawn::Vec3 oxygenOffset = Dawn::Vec3(0, 3, 0);

	float screenTop = 3.5;

	DawnApp() {

		srand(time(NULL));

		Dawn::EventHandler::Listen(Dawn::EventType::KeyPressed, BIND_EVENT_MEMBER_FN(DawnApp::onKeyEvent));

		scene.getMainCamera()->setSize(3.5);

		this->getWindow().setWidth(1200);
		this->getWindow().setHeight(1600);

		scuba.loadFromFile("scuba.png");
		bubbleColumn.loadFromFile("bubble column.png");
		bg.loadFromFile("water bg.png");
		bubble.loadFromFile("bubble.png");

		background = scene.addEntity();
		auto& backgroundTransform = scene.getComponent<Dawn::TransformComponent>(background);
		backgroundTransform.scale.x = 6;
		backgroundTransform.scale.y = 16;
		backgroundTransform.position.y = -4;
		backgroundTransform.position.z = -5;
		scene.addComponent<Dawn::SpriteRendererComponent>(background);
		auto& backgroundSprite = scene.getComponent<Dawn::SpriteRendererComponent>(background);
		backgroundSprite.texture = &bg;

		background2 = scene.addEntity();
		auto& background2Transform = scene.getComponent<Dawn::TransformComponent>(background2);
		background2Transform.scale.x = 6;
		background2Transform.scale.y = 16;
		background2Transform.position.z = -5;
		background2Transform.position.y = -20;
		scene.addComponent<Dawn::SpriteRendererComponent>(background2);
		auto& background2Sprite = scene.getComponent<Dawn::SpriteRendererComponent>(background2);
		background2Sprite.texture = &bg;

		bubbleEntity = scene.addEntity();
		scene.addComponent<Dawn::SpriteRendererComponent>(bubbleEntity);
		auto& bubbleSprite = scene.getComponent<Dawn::SpriteRendererComponent>(bubbleEntity);
		auto& bubbleTransform = scene.getComponent<Dawn::TransformComponent>(bubbleEntity);
		bubbleSprite.texture = &bubble;
		bubbleTransform.position = Dawn::Vec3(randRange(-2.5, 2.5), randRange(0, -15), -2);

		for (int i = 0; i < 5; i++) {
			BubbleColumn * b = new BubbleColumn(scene, &bubbleColumn);
			b->setPosition(Dawn::Vec3(randRange(-2.5, 2.5), randRange(0, -15), -1));
			bubbleColumns.push_back(b);
		}


		scubaEntity = scene.addEntity();
		scene.addComponent<Dawn::SpriteRendererComponent>(scubaEntity);

		auto& spriteRenderer = scene.getComponent<Dawn::SpriteRendererComponent>(scubaEntity);
		spriteRenderer.texture = &scuba;
		auto& transform = scene.getComponent<Dawn::TransformComponent>(scubaEntity);
		transform.position.y = PLAYER_OFFSET;
		transform.rotation = 3.14;

		oxygenEntity = scene.addEntity();
		scene.addComponent<Dawn::SpriteRendererComponent>(oxygenEntity);
		auto& oxygenTransform = scene.getComponent<Dawn::TransformComponent>(oxygenEntity);
		auto& oxygenSpriteRenderer = scene.getComponent<Dawn::SpriteRendererComponent>(oxygenEntity);

		oxygenTransform.position = Dawn::Vec3(0, 0, 1);
		oxygenTransform.scale = Dawn::Vec3(3, 0.25, 1);

		oxygenSpriteRenderer.color = Dawn::Vec4(0.0, 0.0, 1.0, 1.0);


	}

	void onUpdate() {

		auto& transform = scene.getComponent<Dawn::TransformComponent>(scubaEntity);

		float angle = transform.rotation - 3.14;

		angularVelocity = 0;
		velocity.x = 0;
		if (Dawn::Input::GetKeyDown(Dawn::KeyCode::A)) {
			velocity.x -= 4.0;
			angularVelocity = -1;

			if (angle > 0) {
				angularVelocity *= 3;
			}
		}
		if (Dawn::Input::GetKeyDown(Dawn::KeyCode::D)) {
			velocity.x += 4.0;
			angularVelocity = 1;

			if (angle < 0) {
				angularVelocity *= 3;
			}
		}

		if (angularVelocity == 0 && (angle > .01 || angle < -.01)) {
			if (angle > 0) {
				angularVelocity = -1;
			}
			else {
				angularVelocity = 1;
			}
		}

		if (angle > 0.5) angle = 0.5;
		if (angle < -0.5) angle = -0.5;

		transform.rotation = angle + 3.14;
		transform.rotation += angularVelocity * Dawn::Time::deltaTime;

		if (velocity.y < 0) {
			acceleration.y = 10;
		}
		else {
			acceleration.y = 0;
			velocity.y = 0;
		}

		velocity = velocity + acceleration * Dawn::Time::deltaTime;
		transform.position = (velocity + constVelocity) * Dawn::Time::deltaTime + transform.position;

		if (transform.position.x > 2.5) transform.position.x = 2.5;
		if (transform.position.x < -2.5) transform.position.x = -2.5;

		auto& cameraTransform = scene.getComponent<Dawn::TransformComponent>(cameraEntity);
		if (cameraTransform.position.y > transform.position.y - PLAYER_OFFSET) {
			//cameraVelocity.y = -(cameraTransform.position.y - (transform.position.y - PLAYER_OFFSET));
			cameraVelocity.y = 2.5 * ((transform.position.y - PLAYER_OFFSET) - cameraTransform.position.y);

		}
		else {
			cameraVelocity.y = 0;
		}

		cameraTransform.position = cameraTransform.position + cameraVelocity * Dawn::Time::deltaTime;
		auto& oxygenTransform = scene.getComponent<Dawn::TransformComponent>(oxygenEntity);
		oxygenTransform.position = cameraTransform.position + oxygenOffset;

		auto& backgroundTransform = scene.getComponent<Dawn::TransformComponent>(background);
		auto& background2Transform = scene.getComponent<Dawn::TransformComponent>(background2);

		if (backgroundBit) {
			if (cameraTransform.position.y <= background2Transform.position.y - PLAYER_OFFSET) {
				backgroundTransform.position.y -= 32;
				backgroundBit = false;
				DAWN_LOG("here1");
			}
		}
		else {
			if (cameraTransform.position.y <= backgroundTransform.position.y - PLAYER_OFFSET) {
				background2Transform.position.y -= 32;
				backgroundBit = true;
				DAWN_LOG("here2");

			}
		}


		Dawn::Vec2 playerPos = Dawn::Vec2(transform.position.x, transform.position.y);
		for (auto b : bubbleColumns) {
			auto& bubbleColumnTransform = scene.getComponent<Dawn::TransformComponent>(b->e);
			Dawn::Vec2 bubblePos = Dawn::Vec2(bubbleColumnTransform.position.x, bubbleColumnTransform.position.y);

			if (distance(playerPos, bubblePos) < 0.5) {
				applyImpulse(8);
			}

			if (bubblePos.y > screenTop) {
				b->setPosition(Dawn::Vec3(randRange(-2.5, 2.5), randRange(screenTop - 7, screenTop - 7 - 15), -1));
			}
		}
		auto& bubbleTransform = scene.getComponent<Dawn::TransformComponent>(bubbleEntity);

		Dawn::Vec2 bubblePos = Dawn::Vec2(bubbleTransform.position.x, bubbleTransform.position.y);
		if (distance(playerPos, bubblePos) < 0.5) {
			bubbleTransform.position.y = 5;
		}
		if (bubblePos.y > screenTop) {
			bubbleTransform.position = Dawn::Vec3(randRange(-2.5, 2.5), randRange(screenTop - 7, screenTop - 7 - 15), -2);
		}

		screenTop = cameraTransform.position.y + 3.5;

		scene.onUpdate();
	}

	void onClose() {

	}

	void onKeyEvent(const Dawn::Event& e) {
		const Dawn::KeyPressedEvent & k_e = (const Dawn::KeyPressedEvent&)e;

		if (k_e.getKeyCode() == Dawn::KeyCode::Space) {
			applyImpulse(4);
		}
	}

	void applyImpulse(float impulse) {
		auto& transform = scene.getComponent<Dawn::TransformComponent>(scubaEntity);
		velocity.y = -impulse;
	}
};

int main() {
	DawnApp app = DawnApp();
	app.start();
}