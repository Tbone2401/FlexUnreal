// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#ifndef __ApexClothingUtils_h__
#define __ApexClothingUtils_h__

class USkeletalMeshComponent;
class FPhysScene;

#if WITH_APEX_CLOTHING
struct FSubmeshInfo
{
	int32  SubmeshIndex;
	uint32 VertexCount;
	uint32 TriangleCount;
	uint32 SimulVertexCount;
	uint32 FixedVertexCount;

	bool operator==(const FSubmeshInfo& Other) const
	{
		return (SubmeshIndex		== Other.SubmeshIndex
				&& VertexCount		== Other.VertexCount
				&& TriangleCount	== Other.TriangleCount
				&& SimulVertexCount == Other.SimulVertexCount
				&& FixedVertexCount == Other.FixedVertexCount);
	}
};

struct FClothingBackup
{
	bool						bBackedUp;
	TArray<FClothingAssetData>	ClothingAssets;
	TArray<FSkelMeshSection>	Sections;
	TArray<FSkelMeshChunk>		Chunks;
	TArray<uint32>				IndexBuffer;

	void	Clear()
	{
		bBackedUp = false;
		ClothingAssets.Empty();
		Sections.Empty();
		Chunks.Empty();
	}
};
#endif // #if WITH_APEX_CLOTHING

// Define interface for importing apex clothing asset
namespace ApexClothingUtils
{
	enum EClothUtilRetType
	{
		CURT_Fail = 0,
		CURT_Ok,
		CURT_Cancel
	};

	// Function to restore all clothing section to original mesh section related to specified asset index
	UNREALED_API void RemoveAssetFromSkeletalMesh(USkeletalMesh* SkelMesh, uint32 AssetIndex, bool bRecreateSkelMeshComponent = false);
	// Function to restore clothing section to original mesh section
	UNREALED_API void RestoreOriginalClothingSection(USkeletalMesh* SkelMesh, uint32 LODIndex, uint32 SectionIndex);
	UNREALED_API void RestoreOriginalClothingSectionAllLODs(USkeletalMesh* SkelMesh, uint32 LOD0_SectionIndex);

	// using global variable for backing up clothing data
	UNREALED_API void BackupClothingDataFromSkeletalMesh(USkeletalMesh* SkelMesh);
	UNREALED_API void ReapplyClothingDataToSkeletalMesh(USkeletalMesh* SkelMesh);

	// Find mesh section index in a StaticLODModel by material index
	UNREALED_API int32 FindSectionByMaterialIndex( USkeletalMesh* SkelMesh, uint32 LODIndex, uint32 MaterialIndex );

	UNREALED_API uint32 GetMaxClothSimulVertices();

#if WITH_APEX_CLOTHING

	//if AssetIndex is -1, means newly added asset, otherwise re-import
	UNREALED_API EClothUtilRetType ImportApexAssetFromApexFile(FString& ApexFile,USkeletalMesh* SkelMesh, int32 AssetIndex=-1);
	UNREALED_API bool GetSubmeshInfoFromApexAsset(NxClothingAsset *InAsset, uint32 LODIndex, TArray<FSubmeshInfo>& OutSubmeshInfos);

	//Imported LOD is decided according to bUseLOD in clothing asset
	UNREALED_API bool ImportClothingSectionFromClothingAsset(USkeletalMesh* SkelMesh, uint32 SectionIndex, int32 AssetIndex, int32 AssetSubmeshIndex);

	UNREALED_API void ReImportClothingSectionsFromClothingAsset(USkeletalMesh* SkelMesh);
	UNREALED_API void ReImportClothingSectionFromClothingAsset(USkeletalMesh* SkelMesh, uint32 SectionIndex);

	UNREALED_API void BackupClothingDataFromSkeletalMesh(USkeletalMesh* SkelMesh, FClothingBackup& ClothingBackup);
	UNREALED_API void ReapplyClothingDataToSkeletalMesh(USkeletalMesh* SkelMesh, FClothingBackup& ClothingBackup);
	UNREALED_API int32 GetNumLODs(NxClothingAsset *InAsset);
	UNREALED_API int32 GetNumRenderSubmeshes(NxClothingAsset *InAsset, int32 LODIndex);

	UNREALED_API NxClothingAsset* CreateApexClothingAssetFromBuffer(const uint8* Buffer, int32 BufferSize);

#endif // #if WITH_APEX_CLOTHING
} // namespace ApexClothingUtils

#endif //__ApexClothingUtils_h__
