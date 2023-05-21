# Hoverclock

Hoverclock is a simple, yet customizable floating clock. It is a very lightweight alternative to [Hoverclock-QT](https://github.com/kostoskistefan/hoverclock-qt).

## Dependencies

 * XCB (and XCB Shape API)
 * Cairo
 * Pango 
 * [Vault](https://github.com/kostoskistefan/vault)
 * [Inthreadval](https://github.com/kostoskistefan/inthreadval)

### Build dependencies

 * gcc
 * pkg-config
 * make

## Build from source

Ensure that you have all the dependencies installed.

Next, after cloning the repository, you can run the Makefile, which will compile everything into a single executable file called `hoverclock`.

Step by step:
 1. `git clone https://github.com/kostoskistefan/hoverclock.git`
 2. `cd hoverclock`
 3. `make`
 4. `./hoverclock`
