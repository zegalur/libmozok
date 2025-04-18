// ...

#include <libmozok/filesystem.hpp>

namespace mozok {
namespace app {

class StdFileSystem : public FileSystem {
public:
    StdFileSystem();
    Result getTextFile(const Str& path, Str& out) override;
};

}
}
