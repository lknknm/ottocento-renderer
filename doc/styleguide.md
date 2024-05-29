## Ottocento Renderer Style Guide
Since this is a study project, I am very flexible about Code Style improvements and suggestions. Here are the basic rules to follow in order to style the code and make it readable.


### For the C++ library:
- Functions/Methods inside classes should be named in CamelCase.

Example:
```cpp
void NameYourFunction(char* argument_name) {}
```
- "Disposable" variables such `i, j` inside `for loops` always in lowercase.
- Indentation (Tab) with 4 spaces.
- Allman indentation style convention.
- One-line `if/for/foreach` statements can be left without brackets following the example:
```c
if (closestSphere < 0)
    return Miss(ray);
```
- Functions/Methods must have a dashed-line separator before them, followed by a simple description if possible:
```cpp
//----------------------------------------------------------------------------
// back to the original equation, a + bt that gives us the hit position.
// here we are assuming that the closest hit position will be the 
// smaller value on the quadratic solution for t
Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
    // Function
}
```
- Comments inside the methods are also allowed to better clarify the actual intent of some steps.