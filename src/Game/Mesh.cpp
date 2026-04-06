#include "Mesh.hpp"

#include <DirectXMath.h>
#include <tiny_obj_loader.h>
#include <iostream>
#include <unordered_map>

namespace {
struct VertexHash {
	size_t operator()(const Vertex& v) const noexcept
	{
		const uint32_t* data = reinterpret_cast<const uint32_t*>(&v);
		size_t hash = 1469598103934665603ULL;
		for (size_t i = 0; i < sizeof(Vertex) / sizeof(uint32_t); ++i) {
			hash ^= data[i];
			hash *= 1099511628211ULL;
		}
		return hash;
	}
};

struct VertexEqual {
	bool operator()(const Vertex& a, const Vertex& b) const noexcept
	{
		return a.position == b.position && a.normal == b.normal && a.uv == b.uv && a.color == b.color;
	}
};
}  // namespace

Mesh::Mesh() = default;
Mesh::~Mesh() = default;

static Vertex CreateVertex(const Vector3& pos, const Vector3& normal, const Vector2& uv, const Vector4& color)
{
	Vertex vertex;
	vertex.position = pos;
	vertex.normal = normal;
	vertex.uv = uv;
	vertex.color = color;
	return vertex;
}

bool Mesh::CreateCube(GameContext* context)
{
	auto device = context->GetGraphicsDevice();

	vertices.clear();
	indices.clear();

	const Vector3 positions[] = {
		{-0.5f, -0.5f, -0.5f},
		{0.5f, -0.5f, -0.5f},
		{0.5f, 0.5f, -0.5f},
		{-0.5f, 0.5f, -0.5f},
		{-0.5f, -0.5f, 0.5f},
		{0.5f, -0.5f, 0.5f},
		{0.5f, 0.5f, 0.5f},
		{-0.5f, 0.5f, 0.5f},
	};

	const Vector3 normals[] = {
		{0, 0, -1},
		{0, 0, 1},
		{-1, 0, 0},
		{1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
	};

	const Vector2 uvs[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
	const Vector4 color = Colors::White;

	Vertex faceVertices[24];
	int faceIndex = 0;
	auto addFace = [&](int a, int b, int c, int d, const Vector3& normal) {
		faceVertices[faceIndex++] = CreateVertex(positions[a], normal, uvs[0], color);
		faceVertices[faceIndex++] = CreateVertex(positions[b], normal, uvs[1], color);
		faceVertices[faceIndex++] = CreateVertex(positions[c], normal, uvs[2], color);
		faceVertices[faceIndex++] = CreateVertex(positions[d], normal, uvs[3], color);
	};

	addFace(0, 1, 2, 3, normals[0]);
	addFace(5, 4, 7, 6, normals[1]);
	addFace(4, 0, 3, 7, normals[2]);
	addFace(1, 5, 6, 2, normals[3]);
	addFace(3, 2, 6, 7, normals[4]);
	addFace(4, 5, 1, 0, normals[5]);

	for (int i = 0; i < 24; ++i) {
		vertices.push_back(faceVertices[i]);
	}

	uint32_t faceIndices[] = {
		0,	1,	2,	0,	2,	3,	4,	5,	6,	4,	6,	7,	8,	9,	10, 8,	10, 11,
		12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
	};

	indices.assign(std::begin(faceIndices), std::end(faceIndices));

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	if (FAILED(device->CreateBuffer(&vbd, &initData, &vertexBuffer))) {
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices.data();
	if (FAILED(device->CreateBuffer(&ibd, &initData, &indexBuffer))) {
		return false;
	}

	SetTexture(Texture::CreateWhite(context));
	return true;
}

bool Mesh::CreateSphere(GameContext* context, int slices, int stacks)
{
	auto device = context->GetGraphicsDevice();

	vertices.clear();
	indices.clear();

	for (int stack = 0; stack <= stacks; ++stack) {
		float phi = DirectX::XM_PI * stack / stacks;
		float y = 0.5f * cosf(phi);
		float r = 0.5f * sinf(phi);

		for (int slice = 0; slice <= slices; ++slice) {
			float theta = 2.0f * DirectX::XM_PI * slice / slices;
			float x = r * cosf(theta);
			float z = r * sinf(theta);
			Vector3 position = Vector3(x, y, z);
			Vector3 normal = position;
			normal.Normalize();
			Vector2 uv(static_cast<float>(slice) / slices, static_cast<float>(stack) / stacks);
			Vector4 color = Vector4::Lerp(Colors::Blue, Colors::Red, static_cast<float>(stack) / stacks);
			vertices.push_back(CreateVertex(position, normal, uv, color));
		}
	}

	for (int stack = 0; stack < stacks; ++stack) {
		int rowStart = stack * (slices + 1);
		int nextRowStart = (stack + 1) * (slices + 1);

		for (int slice = 0; slice < slices; ++slice) {
			indices.push_back(rowStart + slice);
			indices.push_back(nextRowStart + slice);
			indices.push_back(rowStart + slice + 1);

			indices.push_back(nextRowStart + slice);
			indices.push_back(nextRowStart + slice + 1);
			indices.push_back(rowStart + slice + 1);
		}
	}

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	if (FAILED(device->CreateBuffer(&vbd, &initData, &vertexBuffer))) {
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices.data();
	if (FAILED(device->CreateBuffer(&ibd, &initData, &indexBuffer))) {
		return false;
	}

	SetTexture(Texture::CreateWhite(context));
	return true;
}

bool Mesh::CreateGrid(GameContext* context, float size, int divisions, const Vector4& color)
{
	if (size <= 0.0f || divisions <= 0) {
		return false;
	}

	auto device = context->GetGraphicsDevice();

	vertices.clear();
	indices.clear();

	float half = size * 0.5f;
	float step = size / static_cast<float>(divisions);

	for (int i = 0; i <= divisions; ++i) {
		float coord = -half + i * step;

		vertices.push_back(CreateVertex(Vector3(-half, 0.0f, coord), Vector3::Up, Vector2(0, 0), color));
		vertices.push_back(CreateVertex(Vector3(half, 0.0f, coord), Vector3::Up, Vector2(1, 0), color));

		vertices.push_back(CreateVertex(Vector3(coord, 0.0f, -half), Vector3::Up, Vector2(0, 1), color));
		vertices.push_back(CreateVertex(Vector3(coord, 0.0f, half), Vector3::Up, Vector2(1, 1), color));
	}

	indices.resize(vertices.size());
	for (uint32_t i = 0; i < indices.size(); ++i) {
		indices[i] = i;
	}

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	if (FAILED(device->CreateBuffer(&vbd, &initData, &vertexBuffer))) {
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices.data();
	if (FAILED(device->CreateBuffer(&ibd, &initData, &indexBuffer))) {
		return false;
	}

	topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	SetTexture(Texture::CreateWhite(context));
	return true;
}

bool Mesh::LoadFromObj(
	GameContext* context, const std::string& filePath, const std::string& baseDir,
	std::shared_ptr<Texture> textureResource
)
{
	auto device = context->GetGraphicsDevice();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(), baseDir.c_str(), true)) {
		std::cerr << "OBJ load error: " << warn << err << std::endl;
		return false;
	}

	if (!textureResource) {
		std::string texturePath;

		for (const auto& shape : shapes) {
			for (int materialId : shape.mesh.material_ids) {
				if (materialId >= 0 && materialId < static_cast<int>(materials.size())) {
					const auto& material = materials[materialId];
					if (!material.diffuse_texname.empty()) {
						texturePath = material.diffuse_texname;
						break;
					}
				}
			}
			if (!texturePath.empty()) {
				break;
			}
		}

		if (texturePath.empty()) {
			for (const auto& material : materials) {
				if (!material.diffuse_texname.empty()) {
					texturePath = material.diffuse_texname;
					break;
				}
			}
		}

		if (!texturePath.empty()) {
			std::string fullTexturePath = texturePath;
			bool isAbsolutePath = !texturePath.empty() && (texturePath[0] == '/' || texturePath[0] == '\\' ||
														   (texturePath.size() > 1 && texturePath[1] == ':'));
			if (!baseDir.empty() && !isAbsolutePath) {
				fullTexturePath = baseDir;
				if (fullTexturePath.back() != '/' && fullTexturePath.back() != '\\') {
					fullTexturePath += '/';
				}
				fullTexturePath += texturePath;
			}

			std::wstring texturePathW(fullTexturePath.begin(), fullTexturePath.end());
			auto materialTexture = std::make_shared<Texture>();
			if (materialTexture->LoadFromFile(context, texturePathW)) {
				textureResource = std::move(materialTexture);
			} else {
				std::wcerr << L"Failed to load material texture: " << texturePathW << std::endl;
			}
		}
	}

	if (!textureResource) {
		textureResource = Texture::CreateWhite(context);
	}
	SetTexture(std::move(textureResource));

	vertices.clear();
	indices.clear();
	std::unordered_map<Vertex, uint32_t, VertexHash, VertexEqual> uniqueVertices;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			if (index.vertex_index >= 0 && (size_t)(3 * index.vertex_index + 2) < attrib.vertices.size()) {
				vertex.position = Vector3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				);
			} else {
				vertex.position = Vector3::Zero;
			}

			if (index.normal_index >= 0 && (size_t)(3 * index.normal_index + 2) < attrib.normals.size()) {
				vertex.normal = Vector3(
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				);
			} else {
				vertex.normal = Vector3::Up;
			}

			if (index.texcoord_index >= 0 && (size_t)(2 * index.texcoord_index + 1) < attrib.texcoords.size()) {
				vertex.uv = Vector2(
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				);
			} else {
				vertex.uv = Vector2::Zero;
			}

			vertex.color = Colors::White;

			if (uniqueVertices.count(vertex) == 0) {
				uint32_t newIndex = vertices.size();
				uniqueVertices[vertex] = newIndex;
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	if (FAILED(device->CreateBuffer(&vbd, &initData, &vertexBuffer))) {
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices.data();
	if (FAILED(device->CreateBuffer(&ibd, &initData, &indexBuffer))) {
		return false;
	}

	return true;
}

void Mesh::SetTopology(D3D11_PRIMITIVE_TOPOLOGY newTopology)
{
	topology = newTopology;
}

void Mesh::SetTexture(std::shared_ptr<Texture> texture_)
{
	texture = std::move(texture_);
}

void Mesh::Draw(GameContext* ctx)
{
	auto context = ctx->GetGraphicsContext();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(topology);

	ID3D11ShaderResourceView* resource = texture ? texture->GetView() : nullptr;
	context->PSSetShaderResources(0, 1, &resource);
	context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}
