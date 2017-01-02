import traceback
print "starting:", TESTS_DIR+"/tests/equinox_vis.py"
try:
    execfile(TESTS_DIR+"/tests/equinox_vis.py")
    run(True)
except Exception, exc:
    traceback.print_exc()
    os._exit(1)

