## Overview

  This is a modern Entity-Component-System (ECS) framework built with C++20 and
later, designed to deliver zero-cost abstractions while offering a simple,
expressive, and efficient programming model.  

  Inspired by the ergonomics of Bevy, this library leverages compile-time
metaprogramming, domain-specific language (DSL), and concepts to provide rich
type-level information without runtime overhead.  

  The core philosophy is: "What you write is what you get." Systems are plain
functions, components are ordinary types, and scheduling is declarative—all
resolved at compile time.

## Key Features

-   Zero-runtime-overhead. No virtual dispatch, no heap allocations for system
dispatch.  
-   Compile-time world & schedule construction. Entire execution list is 
generated at compile time.
-   Declarative scheduling. Express dependencies (`before`, `after`) directly
in system registration.
-   Support for async worlds. Run multiple worlds concurrently with shared or 
independent state.
-   DSL-based ergonomic API. Write systems as natural C++ functions, which
means no boierplate.  

## Core Concepts

### 1. Components, Resources

  A type with `component_tag`/`resource_tag` is a component/resource. Inspired
by bevy, resouce are global data accessible by each system.

```c++
struct position {
    using component_tag = void;
    float x;
    float y;
    float z;
};


struct keyboard {
    using resouce_tag = void;
    // ...
};
template <typename O>
struct input : O {
    using resource_tag = typename O::resouce_tag;
};
```

### 2. Get

  We uses class template `query` or `res` to get component or resource data.
Each type parameter in `query` should be a `query_filter` like `with`. These
requested data could be value, cref or ref.

```c++
void update_time(res<timer&> res) {
    auto [timer] = res;
    timer.update();
}
void movement(query<with<position&, const velocity&>> query,
    res<const timer&> res) {
    auto [timer] = res;
    auto delta = timer.delta();
    for (auto [pos, vel] : query.get()) {
        pos += vel * delta;
    }
}
void echo(query<const name&> query) {
    for (auto [entity, name] : query.get_with_entity()) {
        println("entity {} has name: {}", entity, name);
    }
}
```

### 3. Local

  We call variables could not share between systems as `local`.

```c++
void render_system(
    query<with<const position&, const rotation&, scale, visible>> query,
    local<framebuffer_manager> local) {
    ...
}
```

## World & Schedule Declaration

  The entire application structure is defined declaratively at compile time
using `constexpr` pipelines.

```c++
constexpr auto world = world_desc
    | add_system<pre_update, update_keyboard_input>
    | add_system<update, movement>
    | add_system<update, physics_system, before<movement>>
    | add_system<post_update, render_system>
    | ...;
```

## Design Principles

### Compile-Time Reflection & Metaprogramming

  The framework inspects system signatures at compile time using tempalte
metaprogramming and ADL to:

-   Infer required components and resouces.
-   Process dependency.
-   Generate optimal iteration code (e.g., SoA layout, cache-friendly access).

### Zero-Cost Abstraction

  All abstractions (queries, resouces, stages) compile down to direct memory
access and function calls--no runtime ECS "engine" overhead.

### Ergonomic Syntax via DSL

  The framework enables intuitivej parameter  declarations that look like
standard C++ but carry semantic meaning.

## Conclusion

  This ECS framework brings the productivity of high-level game engines to C++,
without sacrificing performance. By pushing logic to compile time and embracing
modern C++ idioms, it enables developers to write clear, safe, and blazingly
fast data-oriented code.