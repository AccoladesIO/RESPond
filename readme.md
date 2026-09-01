# respond

A command-line Redis client written in C++17. It speaks the Redis serialization
protocol (RESP / RESP3) over a raw TCP socket, renders replies as JSON, and
offers both an interactive REPL and one-shot / scripted execution.

`respond` is a from-scratch implementation — no `hiredis`, no third-party
networking or protocol libraries. The only external dependency is
[GoogleTest](https://github.com/google/googletest), and that is used solely for
the test suite.

---

## Features

- **Full RESP3 reply parsing**, rendered as JSON (pretty-printed by default).
- **Interactive REPL** with command history, autocomplete, and a `clear` screen.
- **One-shot mode** — run a single command straight from the shell.
- **Pipelines** — batch multiple commands in a single round trip.
- **Script execution** — run a file of commands, or a semicolon-separated inline string.
- **IPv4 and IPv6** connections via `getaddrinfo`.
- **Connection resilience** — configurable timeout and retry-with-exponential-backoff.
- **Command autocomplete** for common Redis verbs.
- Records the last-used host/port to `~/.respond.conf`.

### RESP3 types understood by the parser

| Category | Types |
|----------|-------|
| Scalars  | simple string `+`, error `-`, integer `:`, double `,`, big number `(`, boolean `#`, null `_` |
| Strings  | bulk string `$`, verbatim string `=` |
| Aggregates | array `*`, set `~`, map `%`, push `>`, attribute `\|` |

`INFO`-style replies (a section header followed by `key:value` lines) are
detected and rendered as a nested JSON object.

---

## Project structure

```
respond/
├── Makefile                 # build the app and the test suite
├── README.md
├── commands.txt             # example script for --script
├── src/
│   ├── main.cpp             # argument parsing, mode selection
│   ├── CLI.{h,cpp}          # interactive REPL loop
│   ├── RedisClient.{h,cpp}  # TCP connection + send (IPv4/IPv6, timeout, retries)
│   ├── CommandHandler.{h,cpp}   # tokenise input, build RESP requests
│   ├── ResponseParser.{h,cpp}   # parse RESP/RESP3 replies into JSON
│   ├── CommandExecutor.{h,cpp}  # one-shot, pipeline, and script execution
│   ├── Autocomplete.{h,cpp}     # command-name prefix completion
│   ├── Config.{h,cpp}       # persist host/port to ~/.respond.conf
│   └── Utils.{h,cpp}        # trim, socket line reads, JSON pretty-printer
└── tests/
    ├── test_command_handler.cpp
    ├── test_response_parser.cpp
    ├── test_redis_client.cpp
    ├── test_utils.cpp
    ├── test_autocomplete.cpp
    └── test_config.cpp
```

---

## Requirements

- A **C++17 compiler** — `g++` or `clang++`.
- **make**.
- **GoogleTest** — only required to build and run the tests.
- A reachable **Redis server** (or compatible) to actually issue commands against.

### Platform support

`respond` uses POSIX / Berkeley sockets (`<sys/socket.h>`, `<netdb.h>`,
`<unistd.h>`), so it builds and runs on **macOS, Linux, and WSL**. It does **not**
compile against native Windows (which uses Winsock); on Windows, build and run it
inside WSL.

---

## Installation

### 1. Install the toolchain and GoogleTest

**macOS** (Homebrew):

```bash
xcode-select --install          # C++ toolchain + make, if not already present
brew install googletest         # for the test suite
brew install redis              # optional: a local server to test against
```

**Ubuntu / Debian / WSL**:

```bash
sudo apt update
sudo apt install build-essential   # g++ + make
sudo apt install libgtest-dev      # for the test suite
sudo apt install redis-server      # optional: a local server to test against
```

### 2. Clone and build

```bash
git clone <your-repo-url> respond
cd respond
make            # builds the ./redis_cli binary
```

That produces the client executable `./redis_cli` in the project root.

> **Tip:** if you have Redis installed locally, start it with `redis-server`
> (default port `6379`) in another terminal before connecting.

---

## Building

The `Makefile` compiles the shared sources once and links them into both the
app and the test binary.

| Command | Result |
|---------|--------|
| `make` / `make app` | Build the `redis_cli` client (default target). |
| `make all` | Build both the client and the `unit_tests` binary. |
| `make test` | Build and run the full test suite. |
| `make run` | Build, then launch the interactive REPL. |
| `make static` | Build the static library `build/librespond.a`. |
| `make shared` | Build the shared library `build/librespond.so` (`.dylib` on macOS). |
| `make install` | Install headers, libraries, and a pkg-config file under `PREFIX`. |
| `make uninstall` | Remove an installed copy. |
| `make clean` | Remove `build/`, `redis_cli`, and `unit_tests`. |

Compilation uses `-std=c++17 -Wall -Wextra -g` and generates header dependency
files automatically, so editing a header triggers the right recompiles.

---

## Usage

```
./redis_cli [options] [COMMAND ARGS...]
```

With no positional command, `respond` starts the interactive REPL. With a
trailing command, it runs that single command and exits.

### Options

| Flag | Argument | Default | Description |
|------|----------|---------|-------------|
| `-h` | host | `127.0.0.1` | Server hostname or IP (IPv4 or IPv6). |
| `-p` | port | `6379` | Server port. |
| `-t` | seconds | `30` | Socket send/receive timeout. |
| `-r` | count | `3` | Connection attempts before giving up (exponential backoff between tries). |
| `-v` | — | off | Verbose mode. |
| `--script` | path | — | Execute a file of commands as a pipeline, then exit. |
| `--inline` | string | — | Execute semicolon-separated commands as a pipeline, then exit. |

### Interactive REPL

```bash
./redis_cli                       # connect to 127.0.0.1:6379
./redis_cli -h cache.local -p 6380
```

Inside the REPL:

| Command | Alias | Action |
|---------|-------|--------|
| `help` | `h` | Show the command list. |
| `exit` / `quit` | `q` | Leave the REPL. |
| `clear` | `c` | Clear the screen. |
| `history` | `hist` | Show commands entered this session. |
| `clear-history` | `ch` | Empty the session history. |
| `PIPELINE a; b; c` | — | Run several commands in one round trip. |

Anything else is parsed and sent to the server, and the reply is printed as
JSON. A unique command prefix is auto-completed; an ambiguous prefix prints a
`Did you mean:` suggestion list.

```
127.0.0.1:6379> SET greeting "hello world"
"OK"
127.0.0.1:6379> GET greeting
"hello world"
127.0.0.1:6379> PIPELINE SET a 1; SET b 2; MGET a b
0) "OK"
1) "OK"
2) ["1", "2"]
```

### One-shot commands

```bash
./redis_cli SET mykey hello
./redis_cli GET mykey
./redis_cli -h 10.0.0.5 -p 6380 LPUSH tasks build test deploy
```

### Scripts

Run a file (blank lines and lines starting with `#` are ignored):

```bash
./redis_cli --script commands.txt
```

`commands.txt`:

```
SET a 1
SET b 2
MGET a b
DEL a b
```

Or pass commands inline, separated by `;`:

```bash
./redis_cli --inline "SET a 1; SET b 2; MGET a b"
```

---

## Testing

The suite is built with GoogleTest and needs **no running Redis** — socket-based
components are exercised against an in-process loopback server / `socketpair`, so
everything is deterministic and fast.

```bash
make test
```

Coverage spans command tokenising and RESP request building, the full RESP3
reply parser, the JSON pretty-printer and socket line readers, autocomplete, the
config round-trip, and the client's connect/send behaviour (including an IPv6
loopback test that self-skips where IPv6 is unavailable).

To run a subset directly:

```bash
make all
./unit_tests --gtest_filter='ParseResponse.*'
./unit_tests --gtest_list_tests        # list everything
```

---

## Using respond as a library

Beyond the CLI, the core can be built as a C++ library and linked into other
projects. Everything except `main.cpp` is bundled; the public API lives in the
`src/*.h` headers (`CommandHandler`, `RedisClient`, `ResponseParser`,
`CommandExecutor`, `Config`, `Autocomplete`, `Utils`).

### Build the library

```bash
make static     # -> build/librespond.a           (recommended)
make shared     # -> build/librespond.so | .dylib
```

Prefer the **static** library for distribution: it links directly into the
consumer's binary, so their program runs anywhere with no runtime setup. The
**shared** library produces smaller binaries but must be locatable by the
dynamic loader at run time (see the note below).

### Install it

`make install` places the headers, both libraries, and a pkg-config file under
`PREFIX` (default `/usr/local`). Headers are installed under a `respond/`
subdirectory so their generic names don't collide with other libraries.

```bash
sudo make install                 # -> /usr/local
make install PREFIX=$HOME/.local  # no sudo, user-local
make uninstall                    # reverse it (respect the same PREFIX)
```

Installed layout:

```
$(PREFIX)/include/respond/*.h
$(PREFIX)/lib/librespond.a
$(PREFIX)/lib/librespond.so            # or .dylib on macOS
$(PREFIX)/lib/pkgconfig/respond.pc
```

> `PREFIX` is where the library will finally live and is baked into `respond.pc`.
> `DESTDIR` (e.g. `make install DESTDIR=/tmp/pkg`) stages files under a temporary
> root for packaging and does **not** alter the recorded prefix.

### Link against it

With the pkg-config file installed, a consuming project needs no hardcoded paths:

```bash
g++ -std=c++17 myapp.cpp $(pkg-config --cflags --libs respond) -o myapp
```

```cpp
#include <respond/CommandHandler.h>
#include <respond/ResponseParser.h>

int main() {
    auto request = CommandHandler::buildRESPCommand({"SET", "key", "value"});
    // ...
}
```

If `PREFIX` is a non-standard location, point pkg-config at it first:
`export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig`.

When linking the **shared** library from a non-standard prefix, the loader also
needs to find it at run time — either set `LD_LIBRARY_PATH=$PREFIX/lib`
(`DYLD_LIBRARY_PATH` on macOS), add an rpath at link time
(`-Wl,-rpath,$PREFIX/lib`), or just use the static library and avoid the issue.

### A note on distribution

C++ has no stable binary interface across compilers and standard-library
versions, so a prebuilt `.so`/`.dylib` may not link cleanly for everyone. The
robust way to share this is as **source**: consumers clone the repo and run
`make install` on their own machine, which sidesteps ABI concerns entirely.

## How it works

A command entered in the REPL flows through the pieces like this:

1. **`CommandHandler::parseCommand`** tokenises the line (respecting
   double-quoted arguments).
2. **`CommandHandler::buildRESPCommand`** serialises the tokens into a RESP array
   of bulk strings — the wire format Redis expects.
3. **`RedisClient::sendCommand`** writes those bytes to the socket opened by
   `connectToServer` (which resolves the host with `getaddrinfo`, so IPv4 and
   IPv6 are handled transparently, and retries with backoff on failure).
4. **`ResponseParser::parseResponse`** reads the reply byte-by-byte, dispatching
   on the RESP type marker, and builds a JSON string.
5. **`Utils::formatPrettyJson`** indents that JSON for display.

`CommandExecutor` wraps steps 3–5 for non-interactive modes (one-shot, pipeline,
script), and `CLI` drives the interactive loop, layering on history and
autocomplete.

---

## Limitations & roadmap

Known constraints, kept here honestly for anyone reading or extending the code:

- **POSIX only.** No native Windows build; use WSL (see *Platform support*).
- **INFO detection is heuristic.** RESP has no type marker for `INFO` replies, so
  they are identified by shape (a `# ` section header plus a multi-line body).
  Interpreting `INFO` at the command layer instead would remove the guesswork.
- **JSON is assembled by string concatenation** rather than through a value model.
  Splitting "parse RESP into a value tree" from "serialise that tree to JSON"
  would make the parser simpler to test and extend.
- **Some parser paths use unguarded `std::stoi`** on length headers; a malformed
  length from the server can throw. Hardening these is the next planned fix.
- **`CommandExecutor` and `CLI` are not yet unit-tested** — both currently read
  and write fixed streams. Injecting their I/O would make them testable without
  stdout capture.

---

## License

Released under the [MIT License](LICENSE). See the `LICENSE` file for details.