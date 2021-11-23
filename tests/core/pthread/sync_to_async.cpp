#include <iostream>

#include <wasmfs/thread_utils.h>

int main() {
  emscripten::SyncToAsync sync_to_async;

  std::cout << "Perform a synchronous task.\n";

  sync_to_async.invoke([](emscripten::SyncToAsync::Callback resume) {
    std::cout << "  Hello from sync C++\n";
    (*resume)();
  });

  std::cout << "Perform an async task.\n";

  sync_to_async.invoke([](emscripten::SyncToAsync::Callback resume) {
    std::cout << "  Hello from sync C++ before the async\n";

    // Set up async JS, just to prove an async JS callback happens before the
    // async C++.
    EM_ASM({
      setTimeout(function() {
        console.log("  Hello from async JS");
      }, 0);
    });

    // Set up async C++..
    emscripten_async_call([](void* arg) {
      auto resume = (emscripten::SyncToAsync::Callback)arg;
      std::cout << "  Hello from async C++\n";

      // We are done with all the async things we want to do, and can call
      // resume to continue execution on the calling thread.
      (*resume)();
    }, resume, 1);
  });

  std::cout << "Perform another synchronous task, also showing var capture.\n";

  int var = 41;

  sync_to_async.invoke([&](emscripten::SyncToAsync::Callback resume) {
    std::cout << "  Hello again from sync C++, we captured " << var << '\n';
    var++;
    (*resume)();
  });

  std::cout << "Captured var is now " << var << '\n';
  assert(var == 42);

  return 0;
}
