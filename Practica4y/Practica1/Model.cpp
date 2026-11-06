#include "Model.h"
#include <unordered_map>


Model::Model()
{
}

void Model::LoadModel(const std::string & fileName)
{
	Assimp::Importer importer;//					Pasa de Polygons y Quads a triangulos, modifica orden para el origen, generar normales si el  objeto no tiene, trata vértices iguales como 1 solo
	//const aiScene *scene=importer.ReadFile(fileName,aiProcess_Triangulate |aiProcess_FlipUVs|aiProcess_GenSmoothNormals|aiProcess_JoinIdenticalVertices);
	const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	if (!scene)
	{	
		printf("Falló en cargar el modelo: %s \n", fileName.c_str(), importer.GetErrorString());
		return;
	}
	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
	}

void Model::ClearModel()
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		if (MeshList[i])
		{
			delete MeshList[i];
			MeshList[i] = nullptr;

		}
	}

	for (unsigned int i = 0; i < TextureList.size(); i++)
	{
		if (TextureList[i])
		{
			delete TextureList[i];
			TextureList[i] = nullptr;
		}
	}

}

void Model::RenderModel()
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		unsigned int materialIndex = meshTotex[i];
		if (!materialIndex< TextureList.size()&& TextureList[materialIndex])
		{
			TextureList[materialIndex]->UseTexture();
		}
		MeshList[i]->RenderMeshModel();

	}


}


Model::~Model()
{
}

void Model::LoadNode(aiNode * node, const aiScene * scene)
{
	for (unsigned int i = 0; i <node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
{

	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.insert(vertices.end(), { mesh->mVertices[i].x,mesh->mVertices[i].y ,mesh->mVertices[i].z });
		//UV
		if (mesh->mTextureCoords[0])//si tiene coordenadas de texturizado
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y});
		}
		else
		{
			vertices.insert(vertices.end(), { 0.0f,0.0f });
		}
		//Normals importante, las normales son negativas porque la luz interactúa con ellas de esa forma, cómo se vio con el dado/cubo
		
		vertices.insert(vertices.end(), { -mesh->mNormals[i].x,-mesh->mNormals[i].y ,-mesh->mNormals[i].z });
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	MeshModel* newMesh = new MeshModel();
	newMesh->CreateMeshModel(&vertices[0], &indices[0], vertices.size(), indices.size());
	MeshList.push_back(newMesh);
	meshTotex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene * scene)
{
	std::unordered_map<std::string, Texture*> loadedTextures; // Mapa para evitar duplicados
	TextureList.resize(scene->mNumMaterials);
	for (unsigned int i = 0; i < scene ->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];
		TextureList[i] = nullptr;
		if (material->GetTextureCount(aiTextureType_DIFFUSE	))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE,0,&path)==AI_SUCCESS)
			{
				int idx;
				std::string filename;
				if (std::string(path.data).rfind("\\"))
				{
					//printf("entre a 1 / \n");
					idx = std::string(path.data).rfind("\\");//para quitar del path del modelo todo lo que este antes del \ de ubicación de directorio
					filename = std::string(path.data).substr(idx + 1);
				}
				/*else if(std::string(path.data).rfind("\\"))
				{
					printf(" entre a 2 \\  \n");
					idx = std::string(path.data).rfind("\\");
					filename = std::string(path.data).substr(idx + 1);
				}*/
				
				
				std::string tga ="tga";
				std::string png = "png";
				std::size_t existetga = filename.find(tga);
				std::size_t existepng= filename.find(png);
				std::string texPath = std::string("Textures/") + filename;
				// Verifica si ya se cargó esta textura antes
				auto it = loadedTextures.find(texPath);
				if (it != loadedTextures.end())
				{
					TextureList[i] = it->second; // Reutiliza la textura ya cargada
				}
				else
				{
					Texture* newTex = new Texture(texPath.c_str());
					std::string ext = filename.substr(filename.find_last_of('.') + 1);

					bool loaded = false;
					if (ext == "tga" || ext=="png")
					{
						loaded = newTex->LoadTextureA();
					}
					else
					{
						loaded = newTex->LoadTexture();
					}

					if (loaded)
					{
						TextureList[i] = newTex;
						loadedTextures[texPath] = newTex; // Almacena para reutilizarla
					}
					else
					{
						printf("Falló en cargar la Textura :%s\n", texPath.c_str());
						delete newTex;
					}
				}

			/*	TextureList[i] = new Texture(texPath.c_str());
				if (existetga != std::string::npos || existepng != std::string::npos)
				{
					if (!TextureList[i]->LoadTextureA())
					{
						printf("Falló en cargar la Textura :%s\n", texPath);
						delete TextureList[i];
						TextureList[i] = nullptr;
					}
				}
				else
				{
					if (!TextureList[i]->LoadTexture())
					{
						printf("Falló en cargar la Textura :%s\n", texPath);
						delete TextureList[i];
						TextureList[i] = nullptr;
					}
				}*/
			}
		}
		//Si no se pudo cargar la textura o no tiene una textura asignada, asignar una textura por defecto
		if (!TextureList[i])
		{
			TextureList[i] = new Texture("Textures/plain.png"); //textura que se aplicará a los modelos si no tienen textura o la textura no se puede cargar
			TextureList[i]->LoadTextureA();
		}

	}
}
