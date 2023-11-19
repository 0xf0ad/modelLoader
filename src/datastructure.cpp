#if IAMFINISHED
#include "../headers/datastructure.h"
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

void showthatdatastucture(const Assimp::Importer* importer){
	ImGui::Begin("data structure");
	size_t importerCount = importer->GetImporterCount();
	Assimp::IOSystem* io = importer->GetIOHandler();
	Assimp::ProgressHandler* progress = importer->GetProgressHandler();
	
	const aiScene* scene = importer->GetScene();
	uint          s_flags         = scene->mFlags;
	aiNode*       s_rootnode      = scene->mRootNode;
	uint          s_nummeshs      = scene->mNumMeshes;
	aiMesh**      s_meshs         = scene->mMeshes;
	uint          s_nummaterials  = scene->mNumMaterials;
	aiMaterial**  s_materials     = scene->mMaterials;
	uint          s_numAnimations = scene->mNumAnimations;
	aiAnimation** s_animations    = scene->mAnimations;
	uint          s_numtextures   = scene->mNumTextures;
	aiTexture**   s_textures      = scene->mTextures;
	uint          s_numlights     = scene->mNumLights;
	aiLight**     s_lights        = scene->mLights;
	uint          s_numcameras    = scene->mNumCameras;
	aiCamera**    s_cameras       = scene->mCameras;
	aiMetadata*   s_metadata      = scene->mMetaData;
	const char*   s_name          = scene->mName.C_Str();
	uint          s_numskeletons  = scene->mNumSkeletons;
	aiSkeleton**  s_skeletons     = scene->mSkeletons;
	bool          s_hasmeshes     = scene->HasMeshes();
	bool          s_hasmaterials  = scene->HasMaterials();
	bool          s_haslights     = scene->HasLights();
	bool          s_hastextures   = scene->HasTextures();
	bool          s_hascameras    = scene->HasCameras();
	bool          s_hasanimations = scene->HasAnimations();
	bool          s_hasskeletons  = scene->hasSkeletons();	


	const aiImporterDesc* importerDesc = importer->GetImporterInfo(importerCount);
	const char* i_author     = importerDesc->mAuthor;
	const char* i_comments   = importerDesc->mComments;
	const char* i_fileExt    = importerDesc->mFileExtensions;
	const char* i_maintainer = importerDesc->mMaintainer;
	uint        i_flags      = importerDesc->mFlags;
	uint        i_maxmajor   = importerDesc->mMaxMajor;
	uint        i_minmajor   = importerDesc->mMinMajor;
	uint        i_maxminor   = importerDesc->mMaxMinor;
	uint        i_minminor   = importerDesc->mMinMinor;
	const char* i_name       = importerDesc->mName;


	aiMemoryInfo in;
	importer->GetMemoryRequirements(in);
	uint r_animation = in.animations;
	uint r_camera    = in.cameras;
	uint r_light     = in.lights;
	uint r_material  = in.materials;
	uint r_mesh      = in.meshes;
	uint r_node      = in.nodes;
	uint r_texture   = in.textures;
	uint r_total     = in.total;

	ImGui::Text("data structure and variables value");
	ImGui::End();
}
#endif
