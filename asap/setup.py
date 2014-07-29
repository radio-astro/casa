from setup_ext import setup

PKGNAME = "asap"

setup(name = PKGNAME,
      version = '4.2.0a',
      description = 'ATNF Spectral-line Analysis Package',
      author = 'Malte Marquarding',
      author_email = 'Malte.Marquarding@csiro.au',
      url = 'http://svn.atnf.csiro.au/trac/asap',
      keywords = ['radio astronomy', 'spectral-line', 'ATNF'],
      long_description = '''A package to process and analyse spectral-line
data from (ATNF) single-dish telescopes.
''',
      package_dir = { PKGNAME: 'python' },
      packages = [ PKGNAME ],
      scripts = ["bin/asap", "bin/asap_update_data",],
      license = 'GPL',
      install_requires = ["ipython>=0.10", "matplotlib>=0.99", "numpy>=1.3"],
#      setup_requires = [ "scons>=1.0" ],
      )
