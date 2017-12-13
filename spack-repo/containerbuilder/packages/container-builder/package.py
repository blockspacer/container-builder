from spack import *

class ContainerBuilder(CMakePackage):
    """ContainerBuilder Client"""

    homepage = "https://github.com/AdamSimpson/ContainerBuilder"
    url      = "https://github.com/AdamSimpson/ContainerBuilder/archive/master.zip"

    version('master', git="https://code.ornl.gov/olcf/ContainerBuilder.git")

    depends_on('boost@1.65.1+coroutine')
