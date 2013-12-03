#
# CASA Mirlib
#

add_library (
casa_mirlib
bug.c
dio.c
headio.c
hio.c
key.c
maskio.c
pack.c
scrio.c
uvio.c
xyio.c
xyzio.c
)

set_target_properties( casa_mirlib PROPERTIES SOVERSION ${casa_soversion} )
target_link_libraries (casa_mirlib casa_casa)
set_target_properties( casa_mirlib PROPERTIES SOVERSION ${casa_soversion} )

install (
TARGETS casa_mirlib
RUNTIME DESTINATION bin
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib
LIBRARY PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install (
FILES
hio.h
io.h
maxdimc.h
miriad.h
sysdep.h
DESTINATION
include/casacore/mirlib
)

