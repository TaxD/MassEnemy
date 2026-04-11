# MassEnemy

An Unreal Engine 5.7 example project demonstrating how to implement enemy AI using the **Mass Entity Framework**. This repository accompanies a tutorial video series on building scalable, high-performance enemy systems with MassEntity.

## Video

[![Watch the video](https://img.youtube.com/vi/sKxVx-9Qbmg/0.jpg)](https://youtu.be/sKxVx-9Qbmg)

## Overview

This project shows how to spawn and manage large numbers of enemy entities efficiently using UE5's Mass Entity (MassGameplay) plugin. It covers the full lifecycle of hostile entities — spawning, wandering AI, actor synchronization, and death handling — all built on the data-oriented Mass Entity architecture.

## Features

- **Mass Entity Spawning** — Queue-based spawn system supporting both EQS (Environment Query System) and grid-based placement
- **Wander AI Processor** — Lightweight, navigation-aware wandering behavior running as a Mass Processor
- **Death Processor** — Entity destruction with TTL-based cleanup for graceful removal
- **Actor Bridge** — Bidirectional synchronization between Mass entities and traditional `ACharacter` actors via `UMassAgentComponent`
- **LOD-Aware Processing** — Processors respect the Mass LOD system, only simulating entities at appropriate detail levels
- **Configurable Settings** — Developer settings for spawn rates, entity configs, compatible worlds, and grid spacing

## Project Structure

```
Source/MassEnemy/
├── Mass/
│   ├── HostileMassFragments.h        # Fragment definitions (Wander, Status, Death)
│   ├── HostileMassMisc.h             # Enums and spawn request data types
│   ├── HostileMassSettings.h/cpp     # Developer settings (spawn config, entity configs)
│   ├── HostileMassSubsystem.h/cpp    # World subsystem managing spawn queue and entity templates
│   ├── HostileMassCharacter.h/cpp    # ACharacter bridge for Mass entity ↔ Actor sync
│   ├── HostileWanderProcessor.h/cpp  # Mass Processor for entity wandering behavior
│   └── HostileDeathProcessor.h/cpp   # Mass Processor for entity death and cleanup
├── MassEnemy.h/cpp                   # Game module definition
└── MassEnemy.Build.cs                # Module build configuration
```

## Key Concepts Demonstrated

| Concept | Where |
|---|---|
| Defining custom Fragments | `HostileMassFragments.h` |
| Writing a Mass Processor | `HostileWanderProcessor`, `HostileDeathProcessor` |
| Spawning entities from a subsystem | `HostileMassSubsystem` |
| Deferred entity commands | `KillHostile()`, `ProcessSpawnRequest()` |
| Actor ↔ Entity synchronization | `HostileMassCharacter` |
| EQS integration with Mass spawning | `QueryAllEQS()`, `GetWeightedRandomLocations()` |
| Navigation system projection | `SetMassLocation()` |

## Requirements

- **Unreal Engine 5.7**
- The following plugins (enabled in `.uproject`):
  - `MassGameplay`
  - `GameplayStateTree` (for BP Template)

## Getting Started

1. Clone this repository
2. Open `MassEnemy.uproject` with Unreal Engine 5.7
3. Build and run the project

## License

This project is licensed under the [MIT License](LICENSE.md).