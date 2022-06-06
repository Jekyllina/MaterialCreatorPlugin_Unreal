// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/MaterialFactoryNew.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Modules/ModuleManager.h"

struct MaterialParams
{
	FString Name;	
	FColor Color;
	UTexture2D* Texture = nullptr;
	float Roughness = -1;
	float Metallic = -1;
	bool Emissive = false;
	float Multiplier = 0;
};

class FMaterialCreatorPluginModule : public IModuleInterface, public FSelfRegisteringExec
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	void CreateMaterial(MaterialParams MyParams);
	void ClearValues(MaterialParams& MyParams);
};
