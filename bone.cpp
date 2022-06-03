#include "headers/bone.h"

//bone data
std::vector<Vertex>         vertexToBone;
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

void addBoneData(uint BoneID, float Weight,uint globalVertexID){
	for(unsigned int i = 0; i < (sizeof(vertexToBone[globalVertexID].boneIDs)/sizeof(uint)); i++){
		if(!vertexToBone[globalVertexID].Weights[i]){
			vertexToBone[globalVertexID].boneIDs[i] = BoneID;
			vertexToBone[globalVertexID].Weights[i] = Weight;
			std::cout<<"\t\tbone "<<BoneID<<" weight "<<Weight<<" index "<<i<<'\n';
			return;
		}
	}
	//we should never get here
	assert(false);
}

void loadSingleBone(unsigned int meshIndex, const aiBone* bone){

	std::cout<<"\tBone : "<<' '<<bone->mName.C_Str()<<" ,vertices effected by that bone : "<<bone->mNumWeights<<'\n';

	int boneID = getBoneID(bone);
	std::cout<<"bone id : "<<boneID<<'\n';

	for (unsigned int i = 0; i < bone->mNumWeights; i++) {
		if (i == 0) std::cout<<'\n';
		const aiVertexWeight& vw = bone->mWeights[i];

		uint globalVertexID = meshBaseVertex[meshIndex] + vw.mVertexId;
		std::cout<<'\t'<<i<<" : vertex id "<<vw.mVertexId<<" weight : "<<vw.mWeight<<"\n";

		assert(globalVertexID < vertexToBone.size());
		addBoneData(boneID, vw.mWeight, globalVertexID);
	}
	std::cout<<'\n';
}




void loadBones(int meshIndex, const aiMesh* mesh){
	for (unsigned int i = 0 ; i < mesh->mNumBones ; i++) {
		loadSingleBone(meshIndex, mesh->mBones[i]);
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
