# Inthreadval

Inthreadval is a simple cross-platform C library for executing user defined functions at regular intervals. It provides a convenient way to run background tasks in a separate thread, without blocking the main thread of the program.

## Example

The following example consists of the following steps:

 1. Create a new Inthreadval instance that will call the `callback` function each second (defined by `INTERVAL_IN_MILLISECONDS`)
 2. Starts the Inthreadval instance, which starts calling the `callback` function at the requested interval
 3. Puts the main thread to sleep for 5 seconds, while the other thread keeps running at regular intervals
 4. After the 5 second sleep, Inthreadval is stopped, which will no longer call the `callback` function
 5. Finally, the Inthreadval instance is destroyed

```c
#ifdef __unix__
    #include <unistd.h>
#elif __WIN32
    #include <windows.h>
    #define sleep(x) Sleep(x)
#endif
#include <inthreadval.h>

#define INTERVAL_IN_MILLISECONDS 1000 // 1000 milliseconds = 1 second

void callback(void)
{
    printf("inthreadval::info: Hello, world!\n");
}

int main(int argc, char *argv[])
{
    inthreadval_s inthreadval;
    inthreadval_create(&inthreadval, callback, INTERVAL_IN_MILLISECONDS);

    inthreadval_start(&inthreadval);
    sleep(5);
    inthreadval_stop(&inthreadval);

    inthreadval_destroy(&inthreadval);

    return 0;
}
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

## License

The Inthreadval is licensed under the MIT License. See `LICENSE` for details.
