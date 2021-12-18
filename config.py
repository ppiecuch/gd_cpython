
def can_build(env, platform):
    return env["tools"] or env["module_gd_cpython_enabled"]


def configure(env):
    pass
