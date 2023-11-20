# Insight

Insight is a tiny C library for writing log messages to standard output. There are 3 types of messages: info, warning and error.

The printed messages will contain the file name and line number from where the function was called, which makes it really useful for debugging your code or providing your users with some useful information.

## Example Usage

Source code:
```c
#include <insight.h>

int main(void)
{
    insight_set_project_name("insight_test");

    insight_info("This is an info message");
    insight_warning("This is a warning message");
    insight_error("This is an error message");

    return 0;
}
```

Output:
```
[insight_test::info] ../test/main.c:7: This is an info message
[insight_test::warning] ../test/main.c:8: This is a warning message
[insight_test::error] ../test/main.c:9: This is an error message
```

## Build

### Requirements

 * C compiler (ex. gcc) 
 * meson
 * ninja

### Build Instructions

Run the following commands to build and install the library:

 1. Prepare the build direction: `meson setup build --prefix=/usr --libdir=/usr/lib`
 2. Compile the library: `meson compile -C build`
 3. (Optional) Install the library: `meson install -C build`

If you chose to skip the third step, you'll find the library inside a folder called `build` in the root directory of this project, which can then be linked to your application.
