# OnlineFPS — 联机功能实现详解

> 基于 Unreal Engine 5.6 的第一人称射击游戏，包含 **PVP 对战** 和 **双人合作（Co-op）** 两种联机模式。

---

## 目录

- [1. 架构总览](#1-架构总览)
- [2. 核心类职责](#2-核心类职责)
- [3. PVP 联机模式](#3-pvp-联机模式)
  - [3.1 玩家加入与队伍分配](#31-玩家加入与队伍分配)
  - [3.2 比赛生命周期](#32-比赛生命周期)
  - [3.3 击杀计分系统](#33-击杀计分系统)
  - [3.4 胜负判定](#34-胜负判定)
- [4. Co-op 合作模式](#4-co-op-合作模式)
  - [4.1 玩家连接与比赛启动](#41-玩家连接与比赛启动)
  - [4.2 敌人管理](#42-敌人管理)
  - [4.3 胜负条件](#43-胜负条件)
- [5. 玩家死亡与重生](#5-玩家死亡与重生)
- [6. 伤害系统](#6-伤害系统)
- [7. 服务器权威设计](#7-服务器权威设计)
- [8. 文件结构](#8-文件结构)

---

## 1. 架构总览

```
┌─────────────────────────────────────────────────────┐
│                    Server (Authority)                │
│                                                      │
│  ┌──────────────┐    ┌──────────────┐               │
│  │  CoopGameMode │    │  PVPGameMode  │               │
│  │  (继承 AGameMode)│    │ (继承 ShooterGameMode)│      │
│  └──────┬───────┘    └──────┬───────┘               │
│         │                   │                        │
│         │           ┌───────┴───────┐               │
│         │           │ShooterGameMode  │               │
│         │           │(维护队伍分数表)  │               │
│         │           └───────┬───────┘               │
│         │                   │                        │
│         ▼                   ▼                        │
│  ┌──────────────────────────────────┐                │
│  │     GameMode 关键职责            │                │
│  │  • 玩家连接/断开管理              │                │
│  │  • 队伍分配                       │                │
│  │  • 比赛开始/结束控制              │                │
│  │  • 击杀计分                       │                │
│  │  • 胜负判定                       │                │
│  └──────────────────────────────────┘                │
│                                                      │
│  ┌─────────────┐  ┌──────────────┐                 │
│  │ShooterCharacter│  │  ShooterNPC   │                 │
│  │  (玩家角色)    │  │  (AI 敌人)    │                 │
│  │  血量/死亡     │  │  血量/死亡     │                 │
│  │  武器系统     │  │  武器系统     │                 │
│  └──────┬──────┘  └──────────────┘                 │
│         │                                            │
│         ▼                                            │
│  ┌──────────────────┐                               │
│  │ShooterPlayerController│                            │
│  │  • 输入映射         │                               │
│  │  • UI管理           │                               │
│  │  • 死亡重生         │                               │
│  └──────────────────┘                               │
└─────────────────────────────────────────────────────┘
          │                          │
          ▼                          ▼
     Client Player 1           Client Player 2
```

---

## 2. 核心类职责

| 类名 | 文件位置 | 职责 |
|------|---------|------|
| `ACoopGameMode` | `CoopGameMode.h/.cpp` | Co-op 合作模式游戏管理，2人合作对抗AI敌人 |
| `AShooterGameMode` | `Variant_Shooter/ShooterGameMode.h/.cpp` | 射手模式基类，维护队伍分数表和UI |
| `APVPGameMode` | `Variant_Shooter/PVPGameMode.h/.cpp` | PVP自由死斗模式，继承自 `AShooterGameMode` |
| `AShooterCharacter` | `Variant_Shooter/ShooterCharacter.h/.cpp` | 玩家角色，管理血量、武器、死亡重生 |
| `AShooterNPC` | `Variant_Shooter/AI/ShooterNPC.h/.cpp` | AI敌人，被击杀后给击杀者队伍加分 |
| `AShooterPlayerController` | `Variant_Shooter/ShooterPlayerController.h/.cpp` | 玩家控制器，管理输入、UI和重生逻辑 |
| `AShooterWeapon` | `Variant_Shooter/Weapons/ShooterWeapon.h/.cpp` | 武器基类，处理开火、弹药和投射物生成 |
| `AShooterProjectile` | `Variant_Shooter/Weapons/ShooterProjectile.h/.cpp` | 投射物，处理命中伤害和范围爆炸 |

---

## 3. PVP 联机模式

### 3.1 玩家加入与队伍分配

**文件：** `PVPGameMode.cpp` — `PostLogin()`

```cpp
void APVPGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // 只有服务器才执行游戏逻辑
    if (!HasAuthority()) return;

    // 人数超限就踢出去
    if (GetNumPlayers() > MaxPlayers)
    {
        NewPlayer->ClientReturnToMainMenuWithTextReason(FText::FromString("Server is full."));
        return;
    }

    // 给新加入的玩家分配一个唯一的队伍编号
    if (APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>())
    {
        uint8 AssignedTeamByte = NextTeamByte++;
        PlayerToTeamByte.Add(PS, AssignedTeamByte);
    }

    // 人齐了就开始比赛
    if (!bMatchStarted && GetNumPlayers() >= MaxPlayers)
    {
        StartPVPMatch();
    }
}
```

**关键设计：**

- 每个玩家加入时，通过 `NextTeamByte` 递增分配唯一队伍编号，存入 `PlayerToTeamByte` 映射表（`TMap<TWeakObjectPtr<APlayerState>, uint8>`）。
- 使用 `TWeakObjectPtr` 避免玩家断开后的悬空引用。
- 最大玩家数 `MaxPlayers` 默认为 2，可编辑。
- 玩家离开时从映射表中移除：

```cpp
void APVPGameMode::Logout(AController* Exiting)
{
    // 先保存 PlayerState，因为 Super::Logout 会清掉它
    APlayerState* LeavingPS = Exiting ? Exiting->GetPlayerState<APlayerState>() : nullptr;
    Super::Logout(Exiting);
    if (LeavingPS) PlayerToTeamByte.Remove(LeavingPS);
}
```

### 3.2 比赛生命周期

```
玩家加入 ──► 人数达标 ──► StartPVPMatch() ──► 比赛进行中
                                              │
                    ┌─────────────────────────┤
                    ▼                         ▼
              达到击杀上限              时间耗尽(600秒)
                    │                         │
                    └─────────┬───────────────┘
                              ▼
                        EndMatch()
                              │
                              ▼
                    广播 OnMatchEnded 事件
                              │
                              ▼
                    延迟5秒 ──► ReturnToMainMenu()
```

**比赛开始：**

```cpp
void APVPGameMode::StartPVPMatch()
{
    if (bMatchStarted) return;  // 防止重复开始
    bMatchStarted = true;

    // 启动比赛倒计时（默认600秒 = 10分钟）
    GetWorldTimerManager().SetTimer(MatchTimerHandle, 
        this, &APVPGameMode::OnMatchTimeExpired, MatchDuration, false);
}
```

**比赛结束：**

```cpp
void APVPGameMode::EndMatch()
{
    if (bMatchEnded) return;  // 防止重复结束
    bMatchEnded = true;

    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    // 遍历分数表找出最高分
    for (const auto& Pair : TeamScores)
    {
        if (!bFoundWinner || Pair.Value > WinnerScore)
        {
            WinnerTeamByte = Pair.Key;
            WinnerScore = Pair.Value;
            bFoundWinner = true;
        }
    }

    // 广播结束事件供蓝图UI响应
    OnMatchEnded.Broadcast(WinnerTeamByte, WinnerScore);

    // 延迟5秒后把所有人送回主菜单
    GetWorldTimerManager().SetTimer(TimerHandle, 
        this, &APVPGameMode::ReturnToMainMenu, 5.0f, false);
}
```

### 3.3 击杀计分系统

**文件：** `ShooterGameMode.cpp` + `PVPGameMode.cpp`

击杀计分采用 **模板方法模式**，基类定义默认行为，子类覆盖：

```
ShooterCharacter.Die()
        │
        ▼
ShooterGameMode.OnCharacterKilled(Victim, Killer)   ← 基类默认：给被击杀者队伍加分
        │
        ▼  (override)
PVPGameMode.OnCharacterKilled(Victim, Killer)        ← PVP覆盖：给击杀者队伍加分
        │
        ├── 取击杀者的 PlayerState
        ├── 从 PlayerToTeamByte 查找队伍编号
        └── IncrementTeamScore(TeamByte)
                │
                ▼
        TeamScores.Add(TeamByte, Score)
        ShooterUI->BP_UpdateScore(TeamByte, Score)   ← 更新计分板UI
```

**PVP 模式的击杀计分逻辑：**

```cpp
void APVPGameMode::OnCharacterKilled(AShooterCharacter* Victim, AController* Killer)
{
    if (bMatchEnded) return;

    // 只给击杀者加分（自杀和环境致死不计分）
    if (Killer && Killer != Victim->GetController())
    {
        if (APlayerState* KillerPS = Killer->GetPlayerState<APlayerState>())
        {
            if (const uint8* TeamByte = PlayerToTeamByte.Find(KillerPS))
            {
                IncrementTeamScore(*TeamByte);
            }
        }
    }

    CheckWinCondition();
}
```

**与基类（ShooterGameMode）的区别：**

| | ShooterGameMode（基类） | PVPGameMode（子类） |
|---|---|---|
| 计分对象 | 被击杀者的队伍 | 击杀者的队伍 |
| 用途 | AI NPC 被杀时给玩家队伍加分 | PVP 玩家互杀时给击杀者加分 |
| 防自杀 | 无 | `Killer != Victim->GetController()` |

### 3.4 胜负判定

PVP 模式有 **两种胜利条件**，满足任一即结束比赛：

```cpp
void APVPGameMode::CheckWinCondition()
{
    if (bMatchEnded) return;

    // 条件1：某队伍击杀数达到上限（默认30）
    for (const auto& Pair : TeamScores)
    {
        if (Pair.Value >= KillScoreLimit)
        {
            EndMatch();
            return;
        }
    }
}

void APVPGameMode::OnMatchTimeExpired()
{
    // 条件2：比赛时间耗尽（默认600秒）
    if (bMatchEnded) return;
    EndMatch();
}
```

| 胜利条件 | 默认值 | 可配置 |
|---------|--------|--------|
| 击杀分数上限 | 30 | `KillScoreLimit` (EditAnywhere) |
| 比赛时长 | 600秒 | `MatchDuration` (EditAnywhere) |

---

## 4. Co-op 合作模式

### 4.1 玩家连接与比赛启动

**文件：** `CoopGameMode.cpp`

```cpp
void ACoopGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority()) return;
    bDelayedStart = true;  // 延迟开始：等待玩家加入
}

void ACoopGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!HasAuthority()) return;

    // 超过2人就拒绝
    if (GetNumPlayers() > MaxPlayers)
    {
        NewPlayer->ClientReturnToMainMenuWithTextReason(
            FText::FromString("Server is full (max 2 players)."));
        return;
    }

    AlivePlayerCount = GetNumPlayers();

    // 2名玩家连接后自动开始比赛
    if (AlivePlayerCount == MaxPlayers)
    {
        StartMatch();
    }
}
```

**Co-op 特殊设计：**

- 继承自 `AGameMode`（而非 `AGameModeBase`），可使用 `StartMatch()` 等 UE 内置比赛流程。
- 设置 `bDelayedStart = true`，让比赛等待玩家到齐后才自动开始。
- 玩家数未满时比赛不会启动，避免单人进入后无法正常游戏。
- **任何玩家离开都会结束比赛**（合作模式中途退出视为失败）：

```cpp
void ACoopGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    if (!HasAuthority()) return;

    AlivePlayerCount = GetNumPlayers();

    // 合作模式下，任何玩家离开直接判负
    if (!bMatchEnded)
    {
        bMatchEnded = true;
        OnGameLost.Broadcast();
        // 3秒后返回主菜单
        GetWorldTimerManager().SetTimer(TimerHandle, 
            this, &ACoopGameMode::ReturnToMainMenu, 3.0f, false);
    }
}
```

### 4.2 敌人管理

GameMode 提供了供外部（如AI生成器蓝图）调用的接口：

```cpp
void ACoopGameMode::EnemySpawned()
{
    if (!HasAuthority()) return;
    ActiveEnemyCount++;
}

void ACoopGameMode::EnemyKilled(AController* Killer)
{
    if (!HasAuthority()) return;
    ActiveEnemyCount = FMath::Max(0, ActiveEnemyCount - 1);
    CheckMatchEndCondition();
}

void ACoopGameMode::PlayerDied(APlayerController* Victim)
{
    if (!HasAuthority()) return;
    AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);
    CheckMatchEndCondition();
}
```

### 4.3 胜负条件

```cpp
void ACoopGameMode::CheckMatchEndCondition()
{
    if (bMatchEnded) return;

    // 胜利：所有敌人被消灭
    if (ActiveEnemyCount <= 0)
    {
        bMatchEnded = true;
        OnGameWon.Broadcast();
        // 5秒后返回主菜单
    }

    // 失败：所有玩家死亡
    if (AlivePlayerCount <= 0)
    {
        bMatchEnded = true;
        OnGameLost.Broadcast();
        // 5秒后返回主菜单
    }
}
```

| 结果 | 条件 | 广播事件 | 延迟返回主菜单 |
|-----|------|---------|---------------|
| 胜利 | `ActiveEnemyCount <= 0` | `OnGameWon` | 5秒 |
| 失败 | `AlivePlayerCount <= 0` | `OnGameLost` | 5秒 |
| 失败 | 任一玩家断开连接 | `OnGameLost` | 3秒 |

---

## 5. 玩家死亡与重生

**文件：** `ShooterCharacter.cpp` + `ShooterPlayerController.cpp`

死亡重生采用 **"销毁角色 → 控制器监听 → 重新生成"** 的设计模式：

```
TakeDamage() ──► 血量归零 ──► Die()
                                │
                   ┌────────────┤
                   ▼            ▼
            通知GameMode     延迟RespawnTime秒
            处理击杀计分       │
                              ▼
                         Destroy() 销毁角色
                              │
                              ▼
              ShooterPlayerController::OnPawnDestroyed()
                              │
                   ┌──────────┤
                   ▼          ▼
            重置弹药UI    随机选出生点
                              │
                              ▼
                    SpawnActor<CharacterClass>()
                              │
                              ▼
                       Possess(新角色)
```

**角色死亡逻辑：**

```cpp
void AShooterCharacter::Die()
{
    // 停用当前武器
    if (IsValid(CurrentWeapon)) CurrentWeapon->DeactivateWeapon();

    // 通知GameMode处理击杀计分
    if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnCharacterKilled(this, LastDamageInstigator.Get());
    }

    GetCharacterMovement()->StopMovementImmediately();
    DisableInput(nullptr);
    OnBulletCountUpdated.Broadcast(0, 0);  // 重置弹药UI
    BP_OnDeath();  // 蓝图处理死亡效果

    // 延迟重生（默认5秒）
    GetWorld()->GetTimerManager().SetTimer(RespawnTimer, 
        this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
    Destroy();  // 销毁角色，触发控制器的 OnDestroyed 事件
}
```

**控制器重生逻辑：**

```cpp
void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
    if (BulletCounterUI) BulletCounterUI->BP_UpdateBulletCounter(0, 0);

    // 防止世界正在销毁时重生
    if (!IsValid(GetWorld()) || GetWorld()->bIsTearingDown) return;

    // 找到所有出生点，随机选一个
    TArray<AActor*> ActorList;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

    if (ActorList.Num() > 0)
    {
        AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];
        const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

        if (AShooterCharacter* RespawnedCharacter = 
            GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
        {
            Possess(RespawnedCharacter);  // 操控新角色
        }
    }
}
```

**设计要点：**

- 角色死亡后不直接重生，而是先销毁旧角色，再由控制器在随机出生点重新生成。
- `OnPossess` 时订阅新角色的委托事件（弹药更新、受伤），并添加 `Player` 标签供AI感知识别。
- 使用 `LastDamageInstigator`（`TWeakObjectPtr`）记录最后伤害来源，避免对方退出后悬空。

---

## 6. 伤害系统

**文件：** `ShooterProjectile.cpp` + `ShooterCharacter.cpp` + `ShooterNPC.cpp`

### 伤害流程

```
投射物命中 ──► NotifyHit()
                  │
          ┌───────┴───────┐
          ▼               ▼
    单体命中模式      爆炸模式
    ProcessHit()    ExplosionCheck()
          │               │
          ▼               ▼
    ApplyDamage()   遍历重叠Actor
                        │
                        ▼
                  ProcessHit() 对每个Actor
                        │
                        ▼
                  ApplyDamage()
```

### 关键参数

| 参数 | 位置 | 默认值 | 说明 |
|------|------|--------|------|
| `HitDamage` | `ShooterProjectile` | 25.0 | 单次命中伤害 |
| `ExplosionRadius` | `ShooterProjectile` | 500cm | 爆炸范围 |
| `bExplodeOnHit` | `ShooterProjectile` | false | 是否爆炸 |
| `bDamageOwner` | `ShooterProjectile` | false | 是否伤害射击者本人 |
| `MaxHP` | `ShooterCharacter` | 500.0 | 玩家最大血量 |
| `CurrentHP` | `ShooterNPC` | 100.0 | NPC最大血量 |

### 伤害来源追踪

```cpp
// ShooterCharacter.cpp — TakeDamage()
LastDamageInstigator = EventInstigator;  // 记录伤害来源（用于击杀计分）
CurrentHP -= Damage;
if (CurrentHP <= 0.0f) Die();
```

```cpp
// ShooterNPC.cpp — Die()
// NPC死亡时直接给击杀者队伍加分（使用基类 ShooterGameMode 的默认行为）
if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
{
    GM->IncrementTeamScore(TeamByte);
}
```

---

## 7. 服务器权威设计

所有联机关键逻辑均在服务器端执行，通过 `HasAuthority()` 守卫：

```cpp
// 所有 GameMode 中的关键函数都有此检查
if (!HasAuthority()) return;
```

### 服务器权威职责划分

| 逻辑 | 执行端 | 说明 |
|------|--------|------|
| 玩家连接/断开 | 服务器 | `PostLogin` / `Logout` |
| 队伍分配 | 服务器 | `PlayerToTeamByte` 映射 |
| 比赛开始/结束 | 服务器 | `StartPVPMatch` / `EndMatch` |
| 击杀计分 | 服务器 | `IncrementTeamScore` / `OnCharacterKilled` |
| 胜负判定 | 服务器 | `CheckWinCondition` / `CheckMatchEndCondition` |
| 返回主菜单 | 服务器→客户端 | `ClientReturnToMainMenuWithTextReason` |
| 伤害判定 | 服务器 | `TakeDamage` |
| 死亡重生 | 客户端 | 角色销毁后由本地控制器重新生成 |
| UI更新 | 客户端 | 通过委托广播触发蓝图更新 |

### 客户端 RPC

```cpp
// 服务器通知客户端返回主菜单
PC->ClientReturnToMainMenuWithTextReason(FText::FromString("Game Over"));
```

### 事件广播（服务器→客户端UI同步）

```cpp
// PVP比赛结束事件
UPROPERTY(BlueprintAssignable, Category = "PVP Events")
FOnPVPMatchEnded OnMatchEnded;  // 广播后蓝图UI更新胜负界面

// Co-op胜负事件
UPROPERTY(BlueprintAssignable, Category = "Game Events")
FOnGameWon OnGameWon;
FOnGameLost OnGameLost;
```

---

## 8. 文件结构

```
Source/OnlineFPS/
│
├── CoopGameMode.h / .cpp          # Co-op 合作模式
│
└── Variant_Shooter/
    │
    ├── PVPGameMode.h / .cpp        # PVP 自由死斗模式
    ├── ShooterGameMode.h / .cpp     # 射手模式基类（队伍分数管理）
    ├── ShooterCharacter.h / .cpp    # 玩家角色（血量/武器/死亡重生）
    ├── ShooterPlayerController.h/.cpp # 玩家控制器（输入/UI/重生）
    │
    ├── AI/
    │   ├── ShooterNPC.h / .cpp      # AI敌人（被杀后给队伍加分）
    │   ├── ShooterAIController.h/.cpp # AI控制器（感知/状态树）
    │   ├── EnvQueryContext_Target.h/.cpp # 环境查询上下文
    │   └── ShooterStateTreeUtility.h/.cpp # 状态树任务和条件
    │
    ├── Weapons/
    │   ├── ShooterWeapon.h / .cpp    # 武器基类（开火/弹药/投射物）
    │   ├── ShooterProjectile.h / .cpp # 投射物（命中/爆炸伤害）
    │   ├── ShooterPickup.h / .cpp    # 武器拾取物
    │   └── ShooterWeaponHolder.h     # 武器持有者接口
    │
    └── UI/
        ├── ShooterUI.h / .cpp         # 计分板UI
        └── ShooterBulletCounterUI.h/.cpp # 弹药/血条UI
```

---

> **引擎版本：** Unreal Engine 5.6  
> **启用插件：** StateTree, GameplayStateTree  
> **网络模式：** Listen Server / Dedicated Server
