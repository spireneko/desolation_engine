#pragma once

#include <random>
#include "GameComponent.hpp"
#include "Texture.hpp"

enum class BlendMode { Additive, AlphaBlend };

struct Particle {
	Vector3 position;
	Vector3 velocity;
	float lifetime;
	float maxLifetime;
	float startSize;
	float endSize;
	float rotation;
	float rotationSpeed;
	Vector4 startColor;
	Vector4 endColor;
	bool active = false;
};

struct EmitterSettings {
	uint32_t maxParticles = 1000;
	float emissionRate = 100.0f;
	float lifetimeMin = 1.0f;
	float lifetimeMax = 3.0f;
	Vector3 velocityMin = Vector3(-1, 0, -1);
	Vector3 velocityMax = Vector3(1, 2, 1);
	float startSize = 1.0f;
	float endSize = 0.0f;
	Vector4 startColor = Colors::White;
	Vector4 endColor = Colors::White;
	float startAlpha = 1.0f;
	float endAlpha = 0.0f;
	bool loop = true;
	Vector3 gravity = Vector3(0, -9.8f, 0);
	std::shared_ptr<Texture> texture;
	BlendMode blendMode = BlendMode::Additive;
	bool useVelocityStretch = false;
	float stretchScale = 0.1f;
	bool localSpace = false;  // true = relative to emitter, false = world space
};

class ParticleEmitterComponent : public GameComponent {
   public:
	ParticleEmitterComponent(GameContext* ctx);
	virtual ~ParticleEmitterComponent() = default;

	void SetSettings(const EmitterSettings& settings);
	const EmitterSettings& GetSettings() const;

	void Update(float deltaTime) override;
	void Draw() override;

	void Emit(uint32_t count);
	void Stop();
	void Reset();

	bool IsPlaying() const { return playing; }

	size_t GetActiveParticleCount() const;

	// For rendering system
	const std::vector<Particle>& GetParticles() const { return particles; }

	BlendMode GetBlendMode() const { return settings.blendMode; }

	std::shared_ptr<Texture> GetTexture() const { return settings.texture; }

   private:
	void SpawnParticle();
	void UpdateParticles(float deltaTime);
	float GetRandomFloat(float min, float max);
	Vector3 GetRandomVector3(const Vector3& min, const Vector3& max);

	EmitterSettings settings;
	std::vector<Particle> particles;

	float emissionAccumulator = 0.0f;
	bool playing = true;
	bool stopped = false;

	std::mt19937 rng;
};
