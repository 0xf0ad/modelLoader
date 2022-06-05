#include "headers/bone.h"
#include <stdio.h>

//bone data
std::vector<Vertex>         vertexToBone;
std::vector<int>            meshBaseVertex;
std::map<std::string, uint> boneNameToIndexMap;
unsigned char tabsNNNum;
void tabNNum(unsigned char tabs){
	tabsNNNum = tabs;
}

void printTabs(unsigned char tabs){
	for(unsigned char i=0; i < tabs; i++){
		printf("\t");
	}
}

void print_assimp_matrix(const aiMatrix4x4& m){
	/*std::cout<<'\t'<<m.a1<<' '<<m.a2<<' '<<m.a3<<' '<<m.a4<<'\n';
	std::cout<<'\t'<<m.b1<<' '<<m.b2<<' '<<m.b3<<' '<<m.b4<<'\n';
	std::cout<<'\t'<<m.c1<<' '<<m.c2<<' '<<m.c3<<' '<<m.c4<<'\n';
	std::cout<<'\t'<<m.d1<<' '<<m.d2<<' '<<m.d3<<' '<<m.d4<<'\n';*/
	printTabs(tabsNNNum); printf("\t%f %f %f %f\n", m.a1, m.a2, m.a3, m.a4);
	printTabs(tabsNNNum); printf("\t%f %f %f %f\n", m.b1, m.b2, m.b3, m.b4);
	printTabs(tabsNNNum); printf("\t%f %f %f %f\n", m.c1, m.c2, m.c3, m.c4);
	printTabs(tabsNNNum); printf("\t%f %f %f %f\n", m.d1, m.d2, m.d3, m.d4);
}

int getBoneID(const aiBone* bone){
	int boneID = 0;
	std::string boneName(bone->mName.C_Str());

	if (boneNameToIndexMap.find(boneName) == boneNameToIndexMap.end()) {
		// Allocate an index for the new bone
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
			//Sstd::cout<<"\t\tbone "<<BoneID<<" weight "<<Weight<<" index "<<i<<'\n';
			return;
		}
	}
	//we should never get here (if we did that means we have some bug)
	assert(false);
}

void loadSingleBone(unsigned int meshIndex, const aiBone* bone){

	std::cout<<"\tBone : "<<' '<<bone->mName.C_Str()<<" ,vertices effected by that bone : "<<bone->mNumWeights<<'\n';

	int boneID = getBoneID(bone);
	//std::cout<<"bone id : "<<boneID<<'\n';

	print_assimp_matrix(bone->mOffsetMatrix);

	for (unsigned int i = 0; i < bone->mNumWeights; i++) {
		if (i == 0) std::cout<<'\n';
		const aiVertexWeight& vw = bone->mWeights[i];

		uint globalVertexID = meshBaseVertex[meshIndex] + vw.mVertexId;
		//std::cout<<'\t'<<i<<" : vertex id "<<vw.mVertexId<<" weight : "<<vw.mWeight<<"\n";

		assert(globalVertexID < vertexToBone.size());
		addBoneData(boneID, vw.mWeight, globalVertexID);
	}
	//std::cout<<'\n';
}

void clearBoneData(Vertex& vertex){
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++){
		vertex.boneIDs[i] = -1;
		vertex.Weights[i] = 0.0f;
	}
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
				
		std::cout<<"Mesh "<<i<<' '<<mesh->mName.C_Str()<<" : vertices "<<Nvertices<<" indices "<<Nindices<<" bones "<<Nbones<<"\n\n";
				
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
