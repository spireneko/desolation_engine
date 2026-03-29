#include "Mesh.hpp"

Mesh::Mesh() = default;
Mesh::~Mesh() = default;

bool Mesh::CreateCube(GameContext* context)
{
	auto device = context->GetGraphicsDevice();

	// Вершины куба (8 точек)
	vertices = {
		{Vector3(-0.5f, -0.5f, -0.5f), Colors::Red},	// 0 красный
		{Vector3(-0.5f, 0.5f, -0.5f), Colors::Green},	// 1 зеленый
		{Vector3(0.5f, 0.5f, -0.5f), Colors::Blue},		// 2 синий
		{Vector3(0.5f, -0.5f, -0.5f), Colors::Yellow},	// 3 желтый
		{Vector3(-0.5f, -0.5f, 0.5f), Colors::Purple},	// 4 пурпурный
		{Vector3(-0.5f, 0.5f, 0.5f), Colors::Cyan},		// 5 бирюзовый
		{Vector3(0.5f, 0.5f, 0.5f), Colors::White},		// 6 белый
		{Vector3(0.5f, -0.5f, 0.5f), Colors::Black},	// 7 черный
	};

	// Индексы (36 для 12 треугольников)
	indices = {
		// Передняя грань
		0,
		2,
		1,
		0,
		3,
		2,
		// Задняя грань
		4,
		5,
		6,
		4,
		6,
		7,
		// Левая грань
		4,
		1,
		5,
		4,
		0,
		1,
		// Правая грань
		3,
		6,
		2,
		3,
		7,
		6,
		// Верхняя грань
		1,
		6,
		5,
		1,
		2,
		6,
		// Нижняя грань
		4,
		3,
		0,
		4,
		7,
		3,
	};

	// Создание буфера вершин
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	if (FAILED(device->CreateBuffer(&vbd, &initData, &vertexBuffer))) {
		return false;
	}

	// Создание буфера индексов
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(WORD) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices.data();
	if (FAILED(device->CreateBuffer(&ibd, &initData, &indexBuffer))) {
		return false;
	}

	return true;
}

void Mesh::Draw(GameContext* ctx)
{
	auto device = ctx->GetGraphicsContext();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	device->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	device->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	device->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}
