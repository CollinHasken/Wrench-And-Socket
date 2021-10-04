#pragma once

class URCStatics;

template<typename AssetClass>
static const AssetClass* URCStatics::GetPrimaryAssetObject(const FPrimaryAssetId& AssetId)
{
	return Cast<AssetClass>(GetPrimaryAssetObject(AssetId));
}
