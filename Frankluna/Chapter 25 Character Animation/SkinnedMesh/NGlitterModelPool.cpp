#include "NGlitterModelPool.h"
#include "../../../NBoxLib/Source/Resource/NAssetGatherer.h"
#include "../../../NBoxLib/Source/Graphics/NTexture2DPool.h"

#include <sstream>


NGlitterModelLoader::NGlitterModelLoader(NGraphicsDevice* dev, NTexture2DPool* texturePool)
	: m_pDevice(dev)
	, m_pTexturePool(texturePool)
{

}

NGlitterModelLoader::~NGlitterModelLoader()
{

}

INAssetLoadResult* NGlitterModelLoader::LoadProxy_GatherThread(INAssetStreamer* streamer
	, INAssetProxy* proxy)
{
	m_pCurWorld = NULL;
	m_pCurObject = NULL;
	m_pCurGroup = NULL;
	m_pCurMat = NULL;

	INStream* content = streamer->GetStream(proxy->GetKey());
	if (content->CheckErr() != INStream::NSTREAM_SUCESS)
	{
		N_ERROR("Cannot load file buffer failed: \nKey = " + proxy->GetKey());
		return NULL;
	}
	UINT64 size = content->ReadByte(0, 0);
	char* buffer = new char[size];
	content->ReadByte(buffer, size);

	std::string objDir = proxy->GetKey();
	auto lastSlash = objDir.find_last_of("\\");
	if (lastSlash == std::string::npos)
	{
		objDir = "";
	}
	else
	{
		objDir = objDir.substr(0, lastSlash + 1);
	}

	m_pCurWorld = new World();
	std::string strBuffer(buffer, buffer + size);
	std::istringstream iss(strBuffer);
	std::string line = "";
	while (std::getline(iss,line))
	{
		consumeOneLine(streamer, line, objDir);
	}
	N_DELETE(content);
	generateTangents(m_pCurWorld);
	generateMatIndexes(m_pCurWorld);

	GlitterModel::World* resultWorld = createWorldFromRawData(m_pCurWorld);

	N_DELETE(m_pCurWorld);
	return new NGlitterModelLoadResult(resultWorld);
}

GlitterModel::World* NGlitterModelLoader::createWorldFromRawData(World* rawWorld)
{
	if (!rawWorld)
		return NULL;

	GlitterModel::World* resultWorld = new GlitterModel::World();
	std::vector<GlitterModel::PosNormalTexTan> vertices;

	// Material
	for (auto matIt = rawWorld->m_vMaterials.begin(); matIt != rawWorld->m_vMaterials.end(); matIt++)
	{
		GlitterModel::Material* mat = new GlitterModel::Material();
		resultWorld->m_vMaterials[matIt->first] = mat;
		mat->m_f3Albedo = matIt->second->m_f3Kd;
		if (matIt->second->m_sMapKd.length() > 0)
			mat->m_sDiffuseMap = m_pTexturePool->GetTexture(matIt->second->m_sDir + matIt->second->m_sMapKd);
		if (matIt->second->m_sBump.length() > 0)
			mat->m_sNormalMap = m_pTexturePool->GetTexture(matIt->second->m_sDir + matIt->second->m_sBump);

		for (auto paraIt = matIt->second->m_vCustomFloats.begin(); paraIt != matIt->second->m_vCustomFloats.end(); paraIt++)
		{
			if (paraIt->first.compare("roughness") == 0)
			{
				mat->m_fRoughness = paraIt->second;
			}
			else if (paraIt->first.compare("specular") == 0)
			{
				mat->m_fSpecular = paraIt->second;
			}
			else if (paraIt->first.compare("metallic") == 0)
			{
				mat->m_fMetallic = paraIt->second;
			}
			else if (paraIt->first.compare("cavity") == 0)
			{
				mat->m_fCavity = paraIt->second;
			}
			else if (paraIt->first.compare("matmask") == 0)
			{
				mat->m_fMatMask = paraIt->second;
			}
			else if (paraIt->first.compare("flakesdensity") == 0)
			{
				mat->m_fFlakesDensity = paraIt->second;
			}
		}
		for (auto paraIt = matIt->second->m_vCustomStrings.begin(); paraIt != matIt->second->m_vCustomStrings.end(); paraIt++)
		{
			if (paraIt->first.compare("rsmcmap") == 0)
			{
				mat->m_sRSMCMap = m_pTexturePool->GetTexture(matIt->second->m_sDir + paraIt->second);
			}
			else if (paraIt->first.compare("matmaskmap") == 0)
			{
				mat->m_sMatMaskMap = m_pTexturePool->GetTexture(matIt->second->m_sDir + paraIt->second);
			}
			else if (paraIt->first.compare("flakesmap") == 0)
			{
				mat->m_sFlakesMap = m_pTexturePool->GetTexture(matIt->second->m_sDir + paraIt->second, NTexture2DPool::TEXTURE2D_TYPE_UINT);
			}
		}
	}

	// Objects
	for (auto objIt = rawWorld->m_vObject.begin(); objIt != rawWorld->m_vObject.end(); objIt++)
	{
		GlitterModel::Object* procObject = new GlitterModel::Object();
		resultWorld->m_vObject[objIt->first] = procObject;
		for (auto grpIt = objIt->second->m_vGroups.begin(); grpIt != objIt->second->m_vGroups.end(); grpIt++)
		{
			GlitterModel::Group* procGroup = new GlitterModel::Group();
			procObject->m_vGroups[grpIt->first] = procGroup;
			for (auto matIt = grpIt->second->m_vMaterials.begin(); matIt != grpIt->second->m_vMaterials.end(); matIt++)
			{
				GlitterModel::GroupMat* procGroupMat = new GlitterModel::GroupMat();
				procGroup->m_vMaterials[matIt->first] = procGroupMat;
				std::vector<UINT> indices;
				for (auto vertIt = matIt->second->m_vVertIndexes.begin(); vertIt != matIt->second->m_vVertIndexes.end(); vertIt++)
				{
					UINT32 indCount = vertIt - matIt->second->m_vVertIndexes.begin();
					for (UINT32 i = 0; i < 3; i++)
					{
						GlitterModel::PosNormalTexTan vertData;
						DirectX::XMFLOAT4 rawVert = rawWorld->m_vVertices[vertIt->p[i]];
						vertData.Pos.x = rawVert.x;
						vertData.Pos.y = rawVert.y;
						vertData.Pos.z = rawVert.z;
						if (matIt->second->m_vNormIndexes.size() > indCount)
						{
							vertData.Normal = rawWorld->m_vNormals[matIt->second->m_vNormIndexes[indCount].p[i]];
						}
						if (matIt->second->m_vTexCIndexes.size() > indCount)
						{
							DirectX::XMFLOAT3 rawTex = rawWorld->m_vTexCoords[matIt->second->m_vTexCIndexes[indCount].p[i]];
							vertData.Tex.x = rawTex.x;
							vertData.Tex.y = rawTex.y;
						}
						if (matIt->second->m_vTanIndexes.size() > indCount && rawWorld->m_vTangents.size() > 0)
						{
							vertData.TangentU = rawWorld->m_vTangents[matIt->second->m_vTanIndexes[indCount].p[i]];
						}
						// check if existed
						INT32 existedIndex = -1;
						//for (auto vertDataIt = vertices.begin(); vertDataIt != vertices.end(); vertDataIt++)
						//{
						//	if (vertDataIt->Pos.x == vertData.Pos.x 
						//		&& vertDataIt->Pos.y == vertData.Pos.y
						//		&& vertDataIt->Pos.z == vertData.Pos.z
						//		&& vertDataIt->Normal.x == vertData.Normal.x
						//		&& vertDataIt->Normal.y == vertData.Normal.y
						//		&& vertDataIt->Normal.z == vertData.Normal.z
						//		&& vertDataIt->Tex.x == vertData.Tex.x
						//		&& vertDataIt->Tex.y == vertData.Tex.y
						//		)
						//	{
						//		existedIndex = vertDataIt - vertices.begin();
						//	}
						//}
						if (existedIndex > 0)
						{
							indices.push_back(existedIndex);
						}
						else
						{
							indices.push_back(vertices.size());
							vertices.push_back(vertData);
						}
					}
				}

				// Create Index Buffer
				if (indices.size() > 0)
				{
					procGroupMat->m_pIndexBuffer = new NIndexBuffer();
					m_pDevice->CreateIndexBuffer(*procGroupMat->m_pIndexBuffer, N_B_USAGE_IMMUTABLE, sizeof(UINT)
						, 0, 0, &indices[0], indices.size(), N_R_FORMAT_R32_UINT, 0);
					procGroupMat->m_uIndexCount = indices.size();
					procGroupMat->m_pMaterial = resultWorld->m_vMaterials[matIt->first];
				}
				else
				{
					procGroupMat->m_pIndexBuffer = NULL;
				}
			}
		}
	}

	// Create Vertex Buffer
	if (vertices.size() > 0)
	{
		resultWorld->m_pVertexBuffer = new NVertexBuffer();
		m_pDevice->CreateVertexBuffer(*resultWorld->m_pVertexBuffer, N_B_USAGE_IMMUTABLE, sizeof(GlitterModel::PosNormalTexTan)
			, 0, 0, &vertices[0], vertices.size(), sizeof(GlitterModel::PosNormalTexTan), 0);
	}
	else
	{
		resultWorld->m_pVertexBuffer = NULL;
	}

	return resultWorld;
}

void NGlitterModelLoader::consumeOneLine(INAssetStreamer* streamer, const std::string &line, const std::string &dir)
{
	if (line.size() <= 0 || line.substr(0, 1).compare("#") == 0)
		return;

	std::istringstream iss(line);

	std::string cmd;
	iss >> cmd;

	if (cmd.compare("mtllib") == 0)
	{
		std::string matlib;
		iss >> matlib;
		matlib = dir + matlib;
		loadMaterialLib(streamer, matlib);
	}
	else if (cmd.compare("o") == 0)
	{
		std::string name;
		iss >> name;
		setObject(name);
	}
	else if (cmd.compare("g") == 0)
	{
		std::string name;
		iss >> name;
		setGroup(name);
	}
	else if (cmd.compare("usemtl") == 0)
	{
		std::string name;
		iss >> name;
		m_sCurMTL = name;
	}
	else if (cmd.compare("v") == 0)
	{
		DirectX::XMFLOAT4 vert;
		iss >> vert.x >> vert.y >> vert.z >> vert.w;
		m_pCurWorld->m_vVertices.push_back(vert);
	}
	else if (cmd.compare("vt") == 0)
	{
		DirectX::XMFLOAT3 vert;
		iss >> vert.x >> vert.y >> vert.z;
		m_pCurWorld->m_vTexCoords.push_back(vert);
	}
	else if (cmd.compare("vn") == 0)
	{
		DirectX::XMFLOAT3 vert;
		iss >> vert.x >> vert.y >> vert.z;
		m_pCurWorld->m_vNormals.push_back(vert);
	}
	else if (cmd.compare("f") == 0)
	{
		if (m_pCurGroup == NULL)
			setGroup("");
		setGroupMat(m_sCurMTL);

		triInd vertIdx;
		triInd normIdx;
		triInd texCIdx;
		UINT32 typeIdx = 0;
		for (int i = 0; i < 4; i++)
		{
			std::string idxChunk;
			iss >> idxChunk;

			if (idxChunk.size() <= 0)
				continue;

			int storeIdx = (i > 2) ? 1 : i;
			if (idxChunk.find("//") != std::string::npos)
			{
				size_t fLength = idxChunk.find("//");
				if (fLength != std::string::npos)
				{
					typeIdx |= 1 | (1 << 1);
					int idxData = std::stoi(idxChunk.substr(0, fLength));
					vertIdx.p[storeIdx] = (idxData < 0) ? m_pCurWorld->m_vVertices.size() + idxData : idxData - 1;
					idxData = std::stoi(idxChunk.substr(fLength + 2));
					normIdx.p[storeIdx] = (idxData < 0) ? m_pCurWorld->m_vNormals.size() + idxData : idxData - 1;
				}
			}
			else if (idxChunk.find("/") != std::string::npos)
			{
				UINT32 fLength = idxChunk.find("/");
				UINT32 sLength = idxChunk.substr(fLength + 1).find("/");
				UINT32 tLength = idxChunk.substr(fLength + sLength + 2).find("/");

				if (fLength != std::string::npos)
				{
					typeIdx |= 1;
					int idxData = std::stoi(idxChunk.substr(0, fLength));
					vertIdx.p[storeIdx] = (idxData < 0) ? m_pCurWorld->m_vVertices.size() + idxData : idxData - 1;
				}

				if (sLength != std::string::npos)
				{
					typeIdx |= 1 << 2;
					int idxData = std::stoi(idxChunk.substr(fLength + 1, sLength));
					texCIdx.p[storeIdx] = (idxData < 0) ? m_pCurWorld->m_vTexCoords.size() + idxData : idxData - 1;
				}

				if (tLength != std::string::npos)
				{
					typeIdx |= 1 << 1;
					int idxData = std::stoi(idxChunk.substr(fLength + sLength + 2, tLength));
					normIdx.p[storeIdx] = (idxData < 0) ? m_pCurWorld->m_vNormals.size() + idxData : idxData - 1;
				}
			}
			else
			{
				typeIdx |= 1;
				int idxData = std::stoi(idxChunk);
				vertIdx.p[storeIdx] = (idxData < 0) ? m_pCurWorld->m_vVertices.size() + idxData : idxData - 1;
			}

			if (i == 2)
			{
				if (typeIdx & 1)
					m_pCurGroupMat->m_vVertIndexes.push_back(vertIdx);
				if (typeIdx & 1 << 1)
					m_pCurGroupMat->m_vNormIndexes.push_back(normIdx);
				if (typeIdx & 1 << 2)
					m_pCurGroupMat->m_vTexCIndexes.push_back(texCIdx);
			}
			else if (i == 3)
			{
				if (typeIdx & 1)
				{
					vertIdx.p[0] ^= vertIdx.p[2] ^= vertIdx.p[0] ^= vertIdx.p[2];
					m_pCurGroupMat->m_vVertIndexes.push_back(vertIdx);
				}
				if (typeIdx & 1 << 1)
				{
					normIdx.p[0] ^= normIdx.p[2] ^= normIdx.p[0] ^= normIdx.p[2];
					m_pCurGroupMat->m_vNormIndexes.push_back(normIdx);
				}
				if (typeIdx & 1 << 2)
				{
					texCIdx.p[0] ^= texCIdx.p[2] ^= texCIdx.p[0] ^= texCIdx.p[2];
					m_pCurGroupMat->m_vTexCIndexes.push_back(texCIdx);
				}
			}
		}

	}
}

void NGlitterModelLoader::loadMaterialLib(INAssetStreamer* streamer, const std::string &file)
{
	INStream* content = streamer->GetStream(file);
	UINT64 size = content->ReadByte(0, 0);
	char* buffer = new char[size];
	if (content->CheckErr() != INStream::NSTREAM_SUCESS)
	{
		N_ERROR("Cannot load file buffer failed: \nKey = " + file);
	}
	content->ReadByte(buffer, size);

	std::string objDir = file;
	auto lastSlash = objDir.find_last_of("\\");
	if (lastSlash == std::string::npos)
	{
		objDir = "";
	}
	else
	{
		objDir = objDir.substr(0, lastSlash + 1);
	}

	std::string strBuffer(buffer, buffer + size);
	std::istringstream iss(strBuffer);
	std::string line = "";
	while (std::getline(iss, line))
	{
		consumeMatOneLine(line, objDir);
	}
	N_DELETE(content);
}

void NGlitterModelLoader::consumeMatOneLine(const std::string &line, const std::string &dir)
{
	if (line.size() <= 0 || line.substr(0, 1).compare("#") == 0)
		return;

	std::istringstream iss(line);

	std::string cmd;
	iss >> cmd;

	if (cmd.compare("newmtl") == 0)
	{
		std::string name;
		iss >> name;
		m_pCurMat = new Material();
		m_pCurWorld->m_vMaterials[name] = m_pCurMat;
		m_pCurMat->m_sDir = dir;
	}
	else if (cmd.compare("Ka") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_f3Ka.x >> m_pCurMat->m_f3Ka.y >> m_pCurMat->m_f3Ka.z;
	}
	else if (cmd.compare("Kd") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_f3Kd.x >> m_pCurMat->m_f3Kd.y >> m_pCurMat->m_f3Kd.z;
	}
	else if (cmd.compare("Ks") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_f3Ks.x >> m_pCurMat->m_f3Ks.y >> m_pCurMat->m_f3Ks.z;
	}
	else if (cmd.compare("Ke") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_f3Ke.x >> m_pCurMat->m_f3Ke.y >> m_pCurMat->m_f3Ke.z;
	}
	else if (cmd.compare("Ns") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_f3Ns.x >> m_pCurMat->m_f3Ns.y >> m_pCurMat->m_f3Ns.z;
	}
	else if (cmd.compare("Ni") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_f3Ni.x >> m_pCurMat->m_f3Ni.y >> m_pCurMat->m_f3Ni.z;
	}
	else if (cmd.compare("d") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_fD;
	}
	else if (cmd.compare("illum") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_u32Illum;
	}
	else if (cmd.compare("map_Ka") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sMapKa;
	}
	else if (cmd.compare("map_Kd") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sMapKd;
	}
	else if (cmd.compare("map_Ks") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sMapKs;
	}
	else if (cmd.compare("map_Ns") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sMapNs;
	}
	else if (cmd.compare("map_d") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sMapD;
	}
	else if (cmd.compare("map_bump") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sMapBump;
	}
	else if (cmd.compare("bump") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sBump;
	}
	else if (cmd.compare("disp") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sDisp;
	}
	else if (cmd.compare("decal") == 0 && m_pCurMat)
	{
		iss >> m_pCurMat->m_sDecal;
	}
	else if (cmd.compare("custom") == 0 && m_pCurMat)
	{
		std::string type;
		std::string name;
		iss >> type >> name;
		if (type.compare("float") == 0)
		{
			iss >> m_pCurMat->m_vCustomFloats[name];
		}
		else if (type.compare("vector") == 0)
		{
			iss >> m_pCurMat->m_vCustomVecs[name].x >> m_pCurMat->m_vCustomVecs[name].y >> m_pCurMat->m_vCustomVecs[name].z;
		}
		else if (type.compare("string") == 0)
		{
			iss >> m_pCurMat->m_vCustomStrings[name];
		}
	}
}

void NGlitterModelLoader::generateTangents(World* world)
{
	struct TAN_SOURCE
	{
		UINT32 vertIndex;
		UINT32 normIndex;
		TAN_SOURCE(UINT32 vert, UINT32 norm) : vertIndex(vert), normIndex(norm) {}
	};

	struct TAN_BI
	{
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
	};

	std::vector<TAN_SOURCE> tangentSources;
	std::vector<TAN_BI> tbResult;
	for (auto objIt = world->m_vObject.begin(); objIt != world->m_vObject.end(); objIt++)
	{
		for (auto grpIt = objIt->second->m_vGroups.begin(); grpIt != objIt->second->m_vGroups.end(); grpIt++)
		{
			for (auto matIt = grpIt->second->m_vMaterials.begin(); matIt != grpIt->second->m_vMaterials.end(); matIt++)
			{
				matIt->second->m_vTanIndexes.resize(matIt->second->m_vVertIndexes.size());
				for (auto vertIt = matIt->second->m_vVertIndexes.begin(); vertIt != matIt->second->m_vVertIndexes.end(); vertIt++)
				{
					UINT32 indCount = vertIt - matIt->second->m_vVertIndexes.begin();
					if (matIt->second->m_vTexCIndexes.size() <= indCount || matIt->second->m_vNormIndexes.size() <= indCount)
						continue;
					DirectX::XMFLOAT4 p0 = world->m_vVertices[vertIt->p[0]];
					DirectX::XMFLOAT4 p1 = world->m_vVertices[vertIt->p[1]];
					DirectX::XMFLOAT4 p2 = world->m_vVertices[vertIt->p[2]];
					DirectX::XMFLOAT3 w0 = world->m_vTexCoords[matIt->second->m_vTexCIndexes[indCount].p[0]];
					DirectX::XMFLOAT3 w1 = world->m_vTexCoords[matIt->second->m_vTexCIndexes[indCount].p[1]];
					DirectX::XMFLOAT3 w2 = world->m_vTexCoords[matIt->second->m_vTexCIndexes[indCount].p[2]];

					float x0 = p1.x - p0.x;
					float x1 = p2.x - p0.x;
					float y0 = p1.y - p0.y;
					float y1 = p2.y - p0.y;
					float z0 = p1.z - p0.z;
					float z1 = p2.z - p0.z;

					float s0 = w1.x - w0.x;
					float s1 = w2.x - w0.x;
					float t0 = w1.y - w0.y;
					float t1 = w2.y - w0.y;

					float r = 1.0f / (s0 * t1 - s1 * t0);
					DirectX::XMFLOAT3 sdir;
					sdir.x = (t1 * x0 - t0 * x1) * r;
					sdir.y = (t1 * y0 - t0 * y1) * r;
					sdir.z = (t1 * z0 - t0 * z1) * r;
					DirectX::XMFLOAT3 tdir;
					tdir.x = (s0 * x1 - s1 * x0) * r;
					tdir.y = (s0 * y1 - s1 * y0) * r;
					tdir.z = (s0 * z1 - s1 * z0) * r;

					for (UINT32 i = 0; i < 3; i++)
					{
						// search for already existed tangents
						INT32 existedIndex = -1;
						//for (auto tSIt = tangentSources.begin(); tSIt != tangentSources.end(); tSIt++)
						//{
						//	if (tSIt->vertIndex == vertIt->p[i] && tSIt->normIndex == grpIt->second->m_vNormIndexes[indCount].p[i])
						//	{
						//		existedIndex = tSIt - tangentSources.begin();
						//	}
						//}
						if (existedIndex > 0)
						{
							tbResult[existedIndex].tangent.x += tdir.x;
							tbResult[existedIndex].tangent.y += tdir.y;
							tbResult[existedIndex].tangent.z += tdir.z;
							tbResult[existedIndex].bitangent.x += sdir.x;
							tbResult[existedIndex].bitangent.y += sdir.y;
							tbResult[existedIndex].bitangent.z += sdir.z;
							matIt->second->m_vTanIndexes[indCount].p[i] = existedIndex;
						}
						else
						{
							matIt->second->m_vTanIndexes[indCount].p[i] = tangentSources.size();
							TAN_SOURCE source = TAN_SOURCE(vertIt->p[i], matIt->second->m_vNormIndexes[indCount].p[i]);
							TAN_BI calResult;
							calResult.tangent = tdir;
							calResult.bitangent = sdir;
							tangentSources.push_back(source);
							tbResult.push_back(calResult);
						}
					}
				}
			}
		}
	}

	// Normalize and store result
	for (auto resultIt = tbResult.begin(); resultIt != tbResult.end(); resultIt++)
	{
		DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&resultIt->tangent);
		DirectX::XMVECTOR b = DirectX::XMLoadFloat3(&resultIt->bitangent);
		DirectX::XMVECTOR n = DirectX::XMLoadFloat3(&world->m_vNormals[tangentSources[resultIt - tbResult.begin()].normIndex]);
		DirectX::XMVECTOR tan = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(t , DirectX::XMVectorMultiply(n , DirectX::XMVector3Dot(n, t))));
		DirectX::XMFLOAT4 tanResult;
		DirectX::XMStoreFloat4(&tanResult, tan);
		DirectX::XMFLOAT3 tempDotResult;
		DirectX::XMStoreFloat3(&tempDotResult, DirectX::XMVector3Dot(DirectX::XMVector3Cross(n, t), b));
		tanResult.w = (tempDotResult.x < 0.f) ? -1.f : 1.f;
		world->m_vTangents.push_back(tanResult);
	}
}

void NGlitterModelLoader::generateMatIndexes(World* world)
{
	//for (auto objIt = world->m_vObject.begin(); objIt != world->m_vObject.end(); objIt++)
	//{
	//	for (auto grpIt = objIt->second->m_vGroups.begin(); grpIt != objIt->second->m_vGroups.end(); grpIt++)
	//	{
	//		for (auto matIt = grpIt->second->m_vMaterials.begin(); matIt != grpIt->second->m_vMaterials.end(); matIt++)
	//		{
	//			if (matIt->second->m_vMatIndexes.size() == 0)
	//			{
	//				MaterialInd matInd;
	//				matInd.m_sMaterialName = "";
	//				matInd.m_uStartIndex = 0;
	//				grpIt->second->m_vMatIndexes.push_back(matInd);
	//			}
	//			for (auto matIt = grpIt->second->m_vMatIndexes.begin(); matIt != grpIt->second->m_vMatIndexes.end(); matIt++)
	//			{
	//				if (matIt + 1 == grpIt->second->m_vMatIndexes.end())
	//				{
	//					matIt->m_uIndexCount = grpIt->second->m_vVertIndexes.size() - matIt->m_uStartIndex;
	//				}
	//				else
	//				{
	//					matIt->m_uIndexCount = (matIt + 1)->m_uStartIndex - matIt->m_uStartIndex;
	//				}
	//			}
	//		}
	//	}
	//}
}

void NGlitterModelLoader::setObject(const std::string &name)
{
	m_pCurObject = m_pCurWorld->m_vObject[name];
	if (m_pCurObject == NULL)
	{
		m_pCurObject = m_pCurWorld->m_vObject[name] = new Object();
	}

	m_pCurGroup = NULL;
}

void NGlitterModelLoader::setGroup(const std::string &name)
{
	if (m_pCurObject == NULL)
		setObject("");

	m_pCurGroup = m_pCurObject->m_vGroups[name];
	if (m_pCurGroup == NULL)
	{
		m_pCurGroup = m_pCurObject->m_vGroups[name] = new Group();
	}

	m_pCurGroupMat = NULL;
}

void NGlitterModelLoader::setGroupMat(const std::string &name)
{
	if (m_pCurGroup == NULL)
		setGroup("");
	m_pCurGroupMat = m_pCurGroup->m_vMaterials[name];
	if (m_pCurGroupMat == NULL)
	{
		m_pCurGroupMat = m_pCurGroup->m_vMaterials[name] = new GroupIndexes();
	}
}

void NGlitterModelLoader::ClearResult(INAssetLoadResult* result)
{
	N_DELETE(result);
}


NGlitterModelPool::NGlitterModelPool(NGraphicsDevice* dev
	, NAssetGatherer* gatherer, NTexture2DPool* texturePool)
	: m_pGatherer(gatherer)
	, m_pTexturePool(texturePool)
{
	m_pModelLoader = new NGlitterModelLoader(dev, texturePool);
	m_pCache = new NFileAssetCache<std::string, NGlitterModelProxy*>();
}

NGlitterModelPool::~NGlitterModelPool()
{
	N_DELETE(m_pModelLoader);
	N_DELETE(m_pCache);
}

void NGlitterModelPool::AddLoadedProxy(INAssetProxy* proxy, INAssetLoadResult* result)
{
	NGlitterModelProxy* modProxy = (NGlitterModelProxy*)proxy;
	NGlitterModelLoadResult* modResult = (NGlitterModelLoadResult*)result;
	if (modProxy)
	{
		if (!modResult || !modResult->GetGlitterModelData())
		{
			modProxy->SetWorld(NULL);
		}
		else
		{
			modProxy->SetWorld(modResult->GetGlitterModelData());
		}
	}
}

NGlitterModelProxy* NGlitterModelPool::GetModel(std::string key, UINT32 type, BOOL async, GlitterModel::World* asset)
{
	NScopedCriticalSection csLock(m_csPoolLock);

	if (m_pCache->IsExist(key))
	{
		NGlitterModelProxy* proxy = m_pCache->GetAsset(key);
		if (asset && proxy->GetWorld() != asset)
		{
			proxy->SetWorld(asset);
		}
		return proxy;
	}
	else
	{
		NGlitterModelProxy* proxy = new NGlitterModelProxy(key);
		m_pCache->AddAsset(key, proxy);
		if (asset)
		{
			proxy->SetWorld(asset);
			return proxy;
		}
		else
		{
			proxy->SetWorld(NULL);
			if (key != "")
			{
				N_ASSERT(m_pGatherer);
				if (async)
				{
					m_pGatherer->AddRequest(proxy, m_pModelLoader, this);
				}
				else
				{
					m_pGatherer->InstantLoadAsset(proxy, m_pModelLoader, this);
				}
			}
			return proxy;
		}
	}
}