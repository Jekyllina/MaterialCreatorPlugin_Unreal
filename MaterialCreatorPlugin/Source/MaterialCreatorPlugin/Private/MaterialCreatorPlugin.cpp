// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialCreatorPlugin.h"
#include "Factories/MaterialFactoryNew.h"
#include "ImageUtils.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FMaterialCreatorPluginModule"

void FMaterialCreatorPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module	
}

bool FMaterialCreatorPluginModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FParse::Command(&Cmd, TEXT("creatematerials")))
	{		
		FString Directory = FPaths::ProjectContentDir();
		FString MyFile = Directory + FParse::Token(Cmd, true);

		TArray<FString> Lines;
		FFileHelper::LoadFileToStringArray(Lines, *MyFile);

		MaterialParams MyParams;		
		
		for (const FString& Line : Lines)
		{		
			if (Line.IsEmpty())
			{
				continue;
			}			

			TArray<FString> MaterialValues;
			Line.ParseIntoArray(MaterialValues, TEXT(" "));

			if (!MaterialValues[0].Compare(TEXT("name"), ESearchCase::IgnoreCase))
			{
				MyParams.Name = MaterialValues[1];
				
				//check if that name already exists		
				FString MatName = FString::Printf(TEXT("/Game/Materials/%s.%s"), *MyParams.Name, *MyParams.Name);

				FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
				FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*MatName);

				if (AssetData.IsValid())
				{
					//change name if it already exists	
					uint64 SuffixAssetName = FPlatformTime::Cycles64();
					MyParams.Name = MyParams.Name + FString::Printf(TEXT("_%llu"), SuffixAssetName);					
				}
			}

			if (!MaterialValues[0].Compare(TEXT("color"), ESearchCase::IgnoreCase) || !MaterialValues[0].Compare(TEXT("emissive"), ESearchCase::IgnoreCase))
			{
				checkf(MaterialValues.Num() == 4 || MaterialValues.Num() == 5, TEXT("\n Error: You have to define at least 3 values for the Color\n"));

				MyParams.Color.R = FCString::Atoi(*MaterialValues[1]);
				MyParams.Color.G = FCString::Atoi(*MaterialValues[2]);
				MyParams.Color.B = FCString::Atoi(*MaterialValues[3]);

				if (MaterialValues.Num() == 4)
				{
					MyParams.Color.A = 255;
				}
				else
				{
					MyParams.Color.A = FCString::Atoi(*MaterialValues[4]);
				}

				if (!MaterialValues[0].Compare(TEXT("emissive"), ESearchCase::IgnoreCase))
				{
					MyParams.Emissive = true;
				}
			}

			if (!MaterialValues[0].Compare(TEXT("mult"), ESearchCase::IgnoreCase))
			{
				MyParams.Multiplier = FCString::Atof(*MaterialValues[1]);
			}

			if (!MaterialValues[0].Compare(TEXT("texture"), ESearchCase::IgnoreCase))
			{
				FString TexDirectory = FPaths::ProjectContentDir();
				FString TexPath = Directory + MaterialValues[1];
				
				TArray<uint8> TextureArray;
				FFileHelper::LoadFileToArray(TextureArray, *TexPath);
				MyParams.Texture = FImageUtils::ImportBufferAsTexture2D(TextureArray);				
			}

			if (!MaterialValues[0].Compare(TEXT("roughness"), ESearchCase::IgnoreCase))
			{
				MyParams.Roughness = FCString::Atof(*MaterialValues[1]);
			}

			if (!MaterialValues[0].Compare(TEXT("metallic"), ESearchCase::IgnoreCase))
			{
				MyParams.Metallic = FCString::Atof(*MaterialValues[1]);
			}

			if (!MaterialValues[0].Compare(TEXT("end"), ESearchCase::IgnoreCase))
			{
				if (MyParams.Name.IsEmpty())
				{
					uint64 SuffixAssetName = FPlatformTime::Cycles64();
					MyParams.Name = FString::Printf(TEXT("Material_%llu"), SuffixAssetName);
				}

				CreateMaterial(MyParams);
				
				ClearValues(MyParams);
			}
		}

		return true;
	}

	return false;	
}

void FMaterialCreatorPluginModule::CreateMaterial(MaterialParams MyParams)
{
	UMaterialFactoryNew* NewMaterialFactory = NewObject<UMaterialFactoryNew>();

		UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Game/Materials/%s"), *MyParams.Name));

		UObject* NewMaterialObject = NewMaterialFactory->FactoryCreateNew(NewMaterialFactory->SupportedClass, Package, *MyParams.Name,
			EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, nullptr, GWarn);

		FAssetRegistryModule::AssetCreated(NewMaterialObject);

		UMaterial* MaterialCasted = Cast<UMaterial>(NewMaterialObject);

		MaterialCasted->Modify();

		UMaterialExpressionVectorParameter* VectorParameter = NewObject<UMaterialExpressionVectorParameter>(MaterialCasted);

		if (MyParams.Emissive)
		{			
			VectorParameter->DefaultValue = FLinearColor(MyParams.Color.R, MyParams.Color.G, MyParams.Color.B, MyParams.Color.A);
			VectorParameter->ParameterName = TEXT("EmissiveColor");
			VectorParameter->MaterialExpressionEditorX = -300;

			UMaterialExpressionMultiply* MaterialMulplier = NewObject<UMaterialExpressionMultiply>(MaterialCasted);
			MaterialMulplier->A.Expression = VectorParameter;
			MaterialMulplier->ConstB = MyParams.Multiplier;
			MaterialMulplier->MaterialExpressionEditorX = -100;

			MaterialCasted->EmissiveColor.Expression = MaterialMulplier;
			MaterialCasted->Expressions.Add(VectorParameter);
			MaterialCasted->Expressions.Add(MaterialMulplier);
		}	
		else
		{			
			VectorParameter->DefaultValue = FLinearColor(MyParams.Color.R, MyParams.Color.G, MyParams.Color.B, MyParams.Color.A);
			VectorParameter->ParameterName = TEXT("BaseColor");
			VectorParameter->MaterialExpressionEditorX = -300;

			MaterialCasted->BaseColor.Expression = VectorParameter;
			MaterialCasted->Expressions.Add(VectorParameter);
		}

		if (MyParams.Texture)
		{
			UMaterialExpressionTextureSampleParameter2D* MyTexture = NewObject<UMaterialExpressionTextureSampleParameter2D>(MaterialCasted);
			MyTexture->Texture = MyParams.Texture;
			MyTexture->MaterialExpressionEditorX = -300;

			MaterialCasted->BaseColor.Expression = MyTexture;
			MaterialCasted->Expressions.Add(MyTexture);	
			MaterialCasted->Expressions.Remove(VectorParameter);
		}

		if (MyParams.Roughness != -1)
		{
			UMaterialExpressionConstant* RoughnessParameter = NewObject<UMaterialExpressionConstant>(MaterialCasted);
			RoughnessParameter->R = MyParams.Roughness;
			RoughnessParameter->MaterialExpressionEditorX = -200;
			RoughnessParameter->MaterialExpressionEditorY = 200;

			MaterialCasted->Roughness.Expression = RoughnessParameter;
			MaterialCasted->Expressions.Add(RoughnessParameter);
		}	

		if (MyParams.Metallic != -1)
		{
			UMaterialExpressionConstant* MetallicParameter = NewObject<UMaterialExpressionConstant>(MaterialCasted);
			MetallicParameter->R = MyParams.Metallic;
			MetallicParameter->MaterialExpressionEditorX = -200;
			MetallicParameter->MaterialExpressionEditorY = 200;

			MaterialCasted->Metallic.Expression = MetallicParameter;
			MaterialCasted->Expressions.Add(MetallicParameter);
		}

		MaterialCasted->PostEditChange();
		MaterialCasted->MarkPackageDirty();
}

void FMaterialCreatorPluginModule::ClearValues(MaterialParams& MyParams)
{
	MyParams.Name = "";
	MyParams.Color = FColor(0, 0, 0);
	MyParams.Texture = nullptr;
	MyParams.Roughness = -1;
	MyParams.Metallic = -1;
	MyParams.Emissive = false;
	MyParams.Multiplier = 0;
}

void FMaterialCreatorPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMaterialCreatorPluginModule, MaterialCreatorPlugin)