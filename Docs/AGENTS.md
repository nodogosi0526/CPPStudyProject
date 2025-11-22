# ドキュメント運用ルール  

# 変数作成時の `Category` 表記ルール**  
- 形式：`Category = "AAA | ～"` を厳守する。  
- 例：  
     ```txt
     Category = "AAA | 近接攻撃AI"
     Category = "AAA | 探索フェーズ"
     Category = "AAA | 武器コンポーネント"
     ```

# 各グループに // --- Animation --- のようにセクションコメントを入れること
- 例：
    // --- Ammo ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 MagazineCapacity = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 MaxTotalAmmo = 120;

	  // --- Muzzle Offset ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	FVector MuzzleOffset = FVector(100.f, 0.f, 10.f);