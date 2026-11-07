#include "Model.h"
#include <unordered_map>


Model::Model()
{
}

void Model::LoadModel(const std::string & fileName,
    bool (*prepararMalla)(aiMesh*, const aiScene*))
{
	Assimp::Importer importer;//					Pasa de Polygons y Quads a triangulos, modifica orden para el origen, generar normales si el  objeto no tiene, trata vértices iguales como 1 solo
	//const aiScene *scene=importer.ReadFile(fileName,aiProcess_Triangulate |aiProcess_FlipUVs|aiProcess_GenSmoothNormals|aiProcess_JoinIdenticalVertices);
	const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        (prepararMalla ? 0 : aiProcess_JoinIdenticalVertices));
	if (!scene)
	{	
		printf("Falló en cargar el modelo: %s \n", fileName.c_str());
		return;
	}
    if (prepararMalla)
    {
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[i];
            if (prepararMalla(mesh, scene))
            {
                delete[] mesh->mNormals;
                mesh->mNormals = nullptr;
            }
        }
        // Las caras corregidas necesitan normales acordes con su nueva forma.
        scene = importer.ApplyPostProcessing(aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
        if (!scene)
        {
            printf("Error al preparar %s: %s\n", fileName.c_str(), importer.GetErrorString());
            return;
        }
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

void Model::RenderModel(GLint colorLocation)
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		unsigned int materialIndex = meshTotex[i];
		if (materialIndex < TextureList.size() && TextureList[materialIndex])
		{
			TextureList[materialIndex]->UseTexture();
		}
        if (colorLocation>=0 && materialIndex<materialColors.size()) {
            auto c=materialColors[materialIndex]; glUniform3f(colorLocation,c.r,c.g,c.b);
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
    TextureList.resize(scene->mNumMaterials,nullptr);
    materialColors.resize(scene->mNumMaterials,aiColor3D(1,1,1));
    for(unsigned int i=0;i<scene->mNumMaterials;++i) {
        auto material=scene->mMaterials[i];
        material->Get(AI_MATKEY_COLOR_DIFFUSE,materialColors[i]);
        aiString path;
        std::string texPath="Textures/plain.png";
        if(material->GetTexture(aiTextureType_DIFFUSE,0,&path)==AI_SUCCESS) {
            std::string name=path.C_Str();
            size_t sep=name.find_last_of("/\\");
            if(sep!=std::string::npos) name=name.substr(sep+1);
            texPath="Textures/"+name;
        }
        TextureList[i]=new Texture(texPath.c_str());
        if(!TextureList[i]->LoadTextureA()) {
            delete TextureList[i]; TextureList[i]=new Texture("Textures/plain.png");
            TextureList[i]->LoadTextureA();
        }
    }
}
