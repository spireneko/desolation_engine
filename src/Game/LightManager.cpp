#include "LightManager.hpp"

#include "PointLight.hpp"

void LightManager::RegisterPointLight(std::shared_ptr<PointLight> light)
{
	if (!light) {
		return;
	}

	// Проверяем, нет ли уже такого света
	for (auto& entry : activeLights) {
		if (auto existing = entry.component.lock()) {
			if (existing.get() == light.get()) {
				return;
			}
		}
	}

	activeLights.push_back({light, 0.0f});
}

void LightManager::UnregisterPointLight(const PointLight* light)
{
	if (!light) {
		return;
	}

	activeLights.erase(
		std::remove_if(
			activeLights.begin(),
			activeLights.end(),
			[light](const LightEntry& entry) {
				auto ptr = entry.component.lock();
				return !ptr || ptr.get() == light;
			}
		),
		activeLights.end()
	);
}

void LightManager::PrepareLights(PerFrameConstants& constants, const Vector3& cameraPosition)
{
	// Очищаем expired указатели и собираем валидные данные
	cachedLights.clear();

	for (auto it = activeLights.begin(); it != activeLights.end();) {
		auto light = it->component.lock();
		if (!light || !light->lightEnabled) {
			it = activeLights.erase(it);
			continue;
		}

		auto data = light->GetLightData();
		it->distanceToCamera = Vector3::DistanceSquared(data.position, cameraPosition);

		cachedLights.push_back(data);
		++it;
	}

	// Сортируем по расстоянию до камеры (ближайшие первыми)
	std::sort(
		cachedLights.begin(),
		cachedLights.end(),
		[&cameraPosition](const LightData::PointLight& a, const LightData::PointLight& b) {
			float distA = Vector3::DistanceSquared(a.position, cameraPosition);
			float distB = Vector3::DistanceSquared(b.position, cameraPosition);
			return distA < distB;
		}
	);

	// Заполняем константный буфер (максимум MAX_POINT_LIGHTS)
	int count = static_cast<int>(std::min<size_t>(cachedLights.size(), MAX_POINT_LIGHTS));
	constants.pointLightCount = count;

	for (int i = 0; i < count; ++i) {
		constants.pointLights[i] = cachedLights[i];
	}

	// Обнуляем оставшиеся слоты (чтобы не было мусора из прошлого кадра)
	for (int i = count; i < MAX_POINT_LIGHTS; ++i) {
		constants.pointLights[i] = LightData::PointLight{};
	}
}
