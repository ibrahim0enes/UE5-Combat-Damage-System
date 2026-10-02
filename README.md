# UE5 Modular Combat & Damage System

A production-ready, highly modular, and scalable combat and damage architecture built in Unreal Engine 5 using a hybrid C++ and Blueprint workflow.

---

## 🛠️️ Architecture & Core Systems

- **Interfaces (`DamageableInterface`)**: Decouples damage logic from specific character classes, allowing any actor implementing the interface to seamlessly receive damage and react.
- **Custom Structs & Enums (`FDamageInfo`)**: Strongly typed data structures encapsulating damage amounts, damage types, instigators, and hit results for clean data passing.
- **Actor Components (`UActorComponent`)**: Encapsulates health and damage processing into reusable components that can be plugged into any player or AI entity without code duplication.
- **Dynamic Multicast Delegates**: Event-driven communication alerting other subsystems (UI, animation, audio) on damage received, healing, and death events.
- **BlueprintNativeEvents**: Core logic implemented in optimized C++ while retaining the flexibility to override or extend functionality directly within Blueprints.
- **Class Inheritance & Extensibility**: Layered hierarchy featuring C++ and Blueprint Base Character classes designed for maintainability and scalability across diverse character types.
