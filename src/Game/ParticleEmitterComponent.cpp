#include "ParticleEmitterComponent.hpp"

ParticleEmitterComponent::ParticleEmitterComponent(GameContext* ctx) : GameComponent(ctx), rng(std::random_device{}())
{
	particles.reserve(settings.maxParticles);
}

void ParticleEmitterComponent::SetSettings(const EmitterSettings& settings_)
{
	settings = settings_;
	particles.reserve(settings.maxParticles);
	// Clamp existing particles if max reduced
	if (particles.size() > settings.maxParticles) {
		particles.resize(settings.maxParticles);
	}
}

const EmitterSettings& ParticleEmitterComponent::GetSettings() const
{
	return settings;
}

void ParticleEmitterComponent::Update(float deltaTime)
{
	if (!isActive) {
		return;
	}

	UpdateParticles(deltaTime);

	if (playing && !stopped) {
		emissionAccumulator += settings.emissionRate * deltaTime;
		uint32_t emitCount = static_cast<uint32_t>(emissionAccumulator);
		if (emitCount > 0) {
			emissionAccumulator -= emitCount;
			Emit(emitCount);
		}
	}

	GameComponent::Update(deltaTime);
}

void ParticleEmitterComponent::UpdateParticles(float deltaTime)
{
	Matrix worldMatrix = GetWorldMatrix();
	Vector3 emitterPos = position;
	if (parent) {
		emitterPos = Vector3::Transform(position, parent->GetWorldMatrix());
	}

	for (auto& p : particles) {
		if (!p.active) {
			continue;
		}

		p.lifetime -= deltaTime;
		if (p.lifetime <= 0.0f) {
			p.active = false;
			continue;
		}

		// Physics
		p.velocity += settings.gravity * deltaTime;
		p.position += p.velocity * deltaTime;
		p.rotation += p.rotationSpeed * deltaTime;
	}

	// Remove inactive particles (swap-pop)
	size_t writeIndex = 0;
	for (size_t i = 0; i < particles.size(); ++i) {
		if (particles[i].active) {
			if (i != writeIndex) {
				particles[writeIndex] = particles[i];
			}
			++writeIndex;
		}
	}
	particles.resize(writeIndex);
}

void ParticleEmitterComponent::SpawnParticle()
{
	if (particles.size() >= settings.maxParticles) {
		return;
	}

	Particle p;
	p.active = true;
	p.lifetime = GetRandomFloat(settings.lifetimeMin, settings.lifetimeMax);
	p.maxLifetime = p.lifetime;
	p.velocity = GetRandomVector3(settings.velocityMin, settings.velocityMax);

	// Local or world space spawn
	if (settings.localSpace) {
		p.position = Vector3::Zero;
	} else {
		p.position = position;
		if (parent) {
			p.position = Vector3::Transform(position, parent->GetWorldMatrix());
		}
	}

	p.startSize = settings.startSize;
	p.endSize = settings.endSize;
	p.rotation = GetRandomFloat(0.0f, DirectX::XM_2PI);
	p.rotationSpeed = GetRandomFloat(-2.0f, 2.0f);

	// Color with alpha
	p.startColor = settings.startColor;
	p.startColor.w = settings.startAlpha;
	p.endColor = settings.endColor;
	p.endColor.w = settings.endAlpha;

	particles.push_back(p);
}

void ParticleEmitterComponent::Emit(uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i) {
		SpawnParticle();
	}
}

void ParticleEmitterComponent::Stop()
{
	stopped = true;
}

void ParticleEmitterComponent::Reset()
{
	particles.clear();
	emissionAccumulator = 0.0f;
	stopped = false;
	playing = true;
}

size_t ParticleEmitterComponent::GetActiveParticleCount() const
{
	size_t count = 0;
	for (const auto& p : particles) {
		if (p.active) {
			++count;
		}
	}
	return count;
}

void ParticleEmitterComponent::Draw()
{
	// Rendering is handled by RenderingSystem in forward pass
	// This override prevents default mesh rendering
}

float ParticleEmitterComponent::GetRandomFloat(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

Vector3 ParticleEmitterComponent::GetRandomVector3(const Vector3& min, const Vector3& max)
{
	return Vector3(GetRandomFloat(min.x, max.x), GetRandomFloat(min.y, max.y), GetRandomFloat(min.z, max.z));
}
