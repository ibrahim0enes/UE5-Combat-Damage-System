# UE5 Combat Damage System

A modular and reusable combat damage system built with Unreal Engine 5 and C++.

## Features

- `DamageableInterface`
- `FDamageInfo` damage struct
- Damage type enums
- Reusable Actor Components
- Health and damage management
- Dynamic Multicast Delegates
- Blueprint integration
- C++ base character architecture
- `BlueprintNativeEvent` support

## Architecture

```text
Attacker
   ↓
Damageable Interface
   ↓
Damage Component
   ↓
Health Component
   ↓
Damage / Health / Death Events
```

## Technologies

- Unreal Engine 5
- C++
- Blueprints
- Git
- Git LFS

## Setup

1. Clone the repository.
2. Run `git lfs pull`.
3. Generate Visual Studio project files.
4. Build the project using `Development Editor`.
5. Open the demo map and press Play.

## Project Status

In development.

## License

MIT License.