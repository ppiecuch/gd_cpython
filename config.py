
def can_build(env, platform):
    return env["tools"] and not env["production"] # for testing only


def configure(env):
    pass
