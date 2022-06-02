#include "headers/bone.h"

//bone data
std::vector<vertexBoneData> vertexToBone;
std::vector<int>            meshBaseVertex;
std::map<std::string, uint> boneNameToIndexMap;

int getBoneID(const aiBone* bone){
	int boneID = 0;
	std::string boneName(bone->mName.C_Str());

	if (boneNameToIndexMap.find(boneName) == boneNameToIndexMap.end()) {
		// Allocate an index for a new bone
		boneID = (int)boneNameToIndexMap.size();
		boneNameToIndexMap[boneName] = boneID;
	}else{
		boneID = boneNameToIndexMap[boneName];
	}
	return boneID;
}

void loadSingleBone(unsigned int meshIndex, const aiBone* pBone){

	std::cout<<"\tBone : "<<' '<<pBone->mName.C_Str()<<" ,vertices effected by that bone : "<<pBone->mNumWeights<<'\n';

	int bone_id = getBoneID(pBone);
	std::cout<<"bone id : "<<bone_id<<'\n';

	for (unsigned int i = 0 ; i < pBone->mNumWeights ; i++) {
		if (i == 0) std::cout<<'\n';
		const aiVertexWeight& vw = pBone->mWeights[i];

		uint globalVertexID = meshBaseVertex[meshIndex] + vw.mVertexId;
		std::cout<<'\t'<<i<<" : vertex id "<<vw.mVertexId<<" weight : "<<vw.mWeight<<'\t';

		assert(globalVertexID < vertexToBone.size());
		vertexToBone[globalVertexID].addBoneData(bone_id, vw.mWeight);
	}
std::cout<<'\n';
}

void loadBones(int mesh_index, const aiMesh* pMesh){
	for (unsigned int i = 0 ; i < pMesh->mNumBones ; i++) {
		loadSingleBone(mesh_index, pMesh->mBones[i]);
	}
}
	
void processBone(const aiScene *scene){
	
	std::cout<<"parsing "<<scene->mNumMeshes<<" meshes\n\n";

	int Tvertices = 0, Tindices = 0, Tbones = 0;
		
	meshBaseVertex.resize(scene->mNumMeshes);

	// process all the scene's meshes (if any)
	for(unsigned int i = 0; i < scene->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[i];
		int Nvertices = mesh->mNumVertices;
		int Nindices = mesh->mNumFaces * 3;
		int Nbones = mesh->mNumBones;
		meshBaseVertex[i] = Tvertices;
				
		std::cout<<"Mesh "<<i<<' '<<mesh->mName.C_Str()<<" : vertices "<<Nvertices<<" indices "<<Nindices<<" bones "<<Nbones<<'\n';
				
		Tvertices += Nvertices;
		Tindices += Nindices;
		Tbones += Nbones;

		vertexToBone.resize(Tvertices);

		if(mesh->HasBones()){
			loadBones(i, mesh);
		}

		std::cout<<'\n';
	}
	std::cout<<"\nTotal vertices : "<<Tvertices<<" Total indices : "<<Tindices<<" Total bones : "<<Tbones<<'\n';
}
