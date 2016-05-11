#include <unity/storage/common/internal/mimetype.h>

#include <unity/storage/common/internal/gobj_memory.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <gio/gio.h>
#pragma GCC diagnostic pop

#include <cassert>

using namespace std;
using namespace unity::storage::common::internal;

string get_mimetype(string const& filename)
{
    string content_type = "application/octet-stream";

    gobj_ptr<GFile> file(g_file_new_for_path(filename.c_str()));
    assert(file);  // Cannot fail according to doc.

    GError* err = nullptr;
    gobj_ptr<GFileInfo> full_info(g_file_query_info(file.get(),
                                                    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                                    G_FILE_QUERY_INFO_NONE,
                                                    /* cancellable */ NULL,
                                                    &err));
    if (!full_info)
    {
        throw runtime_error(err->message);  // LCOV_EXCL_LINE
    }

    content_type = g_file_info_get_attribute_string(full_info.get(), G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
    if (content_type.empty())
    {
        throw runtime_error("get_mimetype(): " + filename + ": could not determine content type");  // LCOV_EXCL_LINE
    }
    return content_type;
}
