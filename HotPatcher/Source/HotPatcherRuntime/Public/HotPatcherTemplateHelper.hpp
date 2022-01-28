#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformMisc.h"
// cpp standard
#include <typeinfo>
#include <cctype>
#include <algorithm>
#include <string>

#if  ENGINE_MAJOR_VERSION <=4 && ENGINE_MINOR_VERSION < 25
#define FProperty UProperty
#endif

namespace THotPatcherTemplateHelper
{
	template<typename ENUM_TYPE>
	static UEnum* GetUEnum()
	{
		SCOPED_NAMED_EVENT_TEXT("GetUEnum",FColor::Red);
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 21
		UEnum* FoundEnum = StaticEnum<ENUM_TYPE>();
#else
		FString EnumTypeName = ANSI_TO_TCHAR(THotPatcherTemplateHelper::GetCPPTypeName<ENUM_TYPE>().c_str());
		UEnum* FoundEnum = FindObject<UEnum>(ANY_PACKAGE, *EnumTypeName, true); 
#endif
		return FoundEnum;
	}
	
	template<typename ENUM_TYPE>
	static FString GetEnumNameByValue(ENUM_TYPE InEnumValue, bool bFullName = false)
	{
		SCOPED_NAMED_EVENT_TEXT("GetEnumNameByValue",FColor::Red);
		FString result;
		{
			FString TypeName;
			FString ValueName;

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 21
			UEnum* FoundEnum = StaticEnum<ENUM_TYPE>();
#else
			FString EnumTypeName = ANSI_TO_TCHAR(THotPatcherTemplateHelper::GetCPPTypeName<ENUM_TYPE>().c_str());
			UEnum* FoundEnum = FindObject<UEnum>(ANY_PACKAGE, *EnumTypeName, true); 
#endif
			if (FoundEnum)
			{
				result = FoundEnum->GetNameByValue((int64)InEnumValue).ToString();
				result.Split(TEXT("::"), &TypeName, &ValueName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				if (!bFullName)
				{
					result = ValueName;
				}
			}
		}
		return result;
	}

	template<typename ENUM_TYPE>
	static bool GetEnumValueByName(const FString& InEnumValueName, ENUM_TYPE& OutEnumValue)
	{
		SCOPED_NAMED_EVENT_TEXT("GetEnumValueByName",FColor::Red);
		bool bStatus = false;

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 21
		UEnum* FoundEnum = StaticEnum<ENUM_TYPE>();
		FString EnumTypeName = FoundEnum->CppType;
#else
		FString EnumTypeName = ANSI_TO_TCHAR(THotPatcherTemplateHelper::GetCPPTypeName<ENUM_TYPE>().c_str());
		UEnum* FoundEnum = FindObject<UEnum>(ANY_PACKAGE, *EnumTypeName, true); 
#endif
		if (FoundEnum)
		{
			FString EnumValueFullName = EnumTypeName + TEXT("::") + InEnumValueName;
			int32 EnumIndex = FoundEnum->GetIndexByName(FName(*EnumValueFullName));
			if (EnumIndex != INDEX_NONE)
			{
				int32 EnumValue = FoundEnum->GetValueByIndex(EnumIndex);
				ENUM_TYPE ResultEnumValue = (ENUM_TYPE)EnumValue;
				OutEnumValue = ResultEnumValue;
				bStatus = true;
			}
		}
		return bStatus;
	}

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 21
	template<typename T>
	static std::string GetCPPTypeName()
	{
		std::string result;
		std::string type_name = typeid(T).name();

		std::for_each(type_name.begin(),type_name.end(),[&result](const char& character){if(!std::isdigit(character)) result.push_back(character);});

		return result;
	}
#endif
	
	template<typename TStructType>
	static bool TSerializeStructAsJsonObject(const TStructType& InStruct,TSharedPtr<FJsonObject>& OutJsonObject)
	{
		SCOPED_NAMED_EVENT_TEXT("TSerializeStructAsJsonObject",FColor::Red);
		if(!OutJsonObject.IsValid())
		{
			OutJsonObject = MakeShareable(new FJsonObject);
		}
		bool bStatus = FJsonObjectConverter::UStructToJsonObject(TStructType::StaticStruct(),&InStruct,OutJsonObject.ToSharedRef(),0,0);
		return bStatus;
	}

	template<typename TStructType>
    static bool TDeserializeJsonObjectAsStruct(const TSharedPtr<FJsonObject>& OutJsonObject,TStructType& InStruct)
	{
		SCOPED_NAMED_EVENT_TEXT("TDeserializeJsonObjectAsStruct",FColor::Red);
		bool bStatus = false;
		if(OutJsonObject.IsValid())
		{
			bStatus = FJsonObjectConverter::JsonObjectToUStruct(OutJsonObject.ToSharedRef(),TStructType::StaticStruct(),&InStruct,0,0);
		}
		return bStatus;
	}

	template<typename TStructType>
    static bool TSerializeStructAsJsonString(const TStructType& InStruct,FString& OutJsonString)
	{
		SCOPED_NAMED_EVENT_TEXT("TSerializeStructAsJsonString",FColor::Red);
		bool bRunStatus = false;

		{
			TSharedPtr<FJsonObject> JsonObject;
			if (THotPatcherTemplateHelper::TSerializeStructAsJsonObject<TStructType>(InStruct,JsonObject) && JsonObject.IsValid())
			{
				auto JsonWriter = TJsonWriterFactory<>::Create(&OutJsonString);
				FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
				bRunStatus = true;
			}
		}
		return bRunStatus;
	}

	template<typename TStructType>
    static bool TDeserializeJsonStringAsStruct(const FString& InJsonString,TStructType& OutStruct)
	{
		SCOPED_NAMED_EVENT_TEXT("TDeserializeJsonStringAsStruct",FColor::Red);
		bool bRunStatus = false;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(InJsonString);
		TSharedPtr<FJsonObject> DeserializeJsonObject;
		if (FJsonSerializer::Deserialize(JsonReader, DeserializeJsonObject))
		{
			bRunStatus = THotPatcherTemplateHelper::TDeserializeJsonObjectAsStruct<TStructType>(DeserializeJsonObject,OutStruct);
		}
		return bRunStatus;
	}

	static TMap<FString, FString> GetCommandLineParamsMap(const FString& CommandLine)
	{
		TMap<FString, FString> resault;
		TArray<FString> ParamsSwitchs, ParamsTokens;
		FCommandLine::Parse(*CommandLine, ParamsTokens, ParamsSwitchs);

		for (const auto& SwitchItem : ParamsSwitchs)
		{
			TArray<FString> SwitchArray;
			SwitchItem.ParseIntoArray(SwitchArray,TEXT("="),true);
			if(SwitchArray.Num()>1)
			{
				resault.Add(SwitchArray[0],SwitchArray[1].TrimQuotes());
			}
			else
			{
				resault.Add(SwitchArray[0],TEXT(""));
			}
		}
		return resault;
	}

	static bool HasPrroperty(UStruct* Field,const FString& FieldName)
	{
		SCOPED_NAMED_EVENT_TEXT("HasPrroperty",FColor::Red);
		for(TFieldIterator<FProperty> PropertyIter(Field);PropertyIter;++PropertyIter)
		{
			if(PropertyIter->GetName().Equals(FieldName,ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	
	template<typename T>
    static void ReplaceProperty(T& Struct, const TMap<FString, FString>& ParamsMap)
	{
		SCOPED_NAMED_EVENT_TEXT("ReplaceProperty",FColor::Red);
		TSharedPtr<FJsonObject> DeserializeJsonObject;
		THotPatcherTemplateHelper::TSerializeStructAsJsonObject(Struct,DeserializeJsonObject);
		if (DeserializeJsonObject.IsValid())
		{
			TArray<FString> MapKeys;
			ParamsMap.GetKeys(MapKeys);

			for(const auto& key:MapKeys)
			{
				TArray<FString> BreakedDot;
				key.ParseIntoArray(BreakedDot,TEXT("."));
				if(BreakedDot.Num())
				{
					TSharedPtr<FJsonObject> JsonObject = DeserializeJsonObject;
					if(HasPrroperty(T::StaticStruct(),BreakedDot[0]))
					{
						for(int32 index=0;index<BreakedDot.Num()-1;++index)
						{
							JsonObject = JsonObject->GetObjectField(BreakedDot[index]);
						}

						if(JsonObject)
						{
							JsonObject->SetStringField(BreakedDot[BreakedDot.Num()-1],*ParamsMap.Find(key));
						}
					}
				}
			}
			THotPatcherTemplateHelper::TDeserializeJsonObjectAsStruct<T>(DeserializeJsonObject,Struct);
		}
	}
	
	template<typename T>
	static TArray<TArray<T>> SplitArray(const TArray<T>& Array,int32 SplitNum)
	{
		SCOPED_NAMED_EVENT_TEXT("SplitArray",FColor::Red);
		TArray<TArray<T>> result;
		result.AddDefaulted(SplitNum);

		for( int32 index=0; index<Array.Num(); ) 
		{
			for(auto& SplitItem:result)
			{
				SplitItem.Add(Array[index]);
				++index;
				if(index >= Array.Num())
				{
					break;
				}
			}
		}
		return result;
	}

	template <typename T>
	TArray<T> GetArrayBySrcWithCondition(TArray<T>& SrcArray, TFunction<bool(T)> Matcher, bool RemoveFromSrc)
	{
		TArray<T> result;
		for(int32 Index = SrcArray.Num() - 1 ;Index >= 0;--Index)
		{
			if(Matcher(SrcArray[Index]))
			{
				result.Add(SrcArray[Index]);
				if(RemoveFromSrc)
				{
					SrcArray.RemoveAtSwap(Index,1,false);
				}
			}
		}
		return result;
	}
}
