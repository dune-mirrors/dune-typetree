TypeTree
========

This is version 2.6.0-rc1 of the TypeTree library.

TypeTree is a template library for constructing and operating on statically
typed trees of objects. It is based around the idea of defining loosely coupled,
componentized algorithms. Component lookup happens through tag dispatch, making
it very easy to extend and / or modify existing algorithms as well as constructing
new algorithms for existing types of tree nodes.

The provided algorithms include visitor-based iteration of both single trees and
pairs of trees and a powerful tree transformation algorithm that automatically
decomposes an existing tree and constructs the transformed one, only requiring
the user to specify the per-node transformation policies.

Moreover, there are some more specialized utility algorithms for compile-time
reductions over trees and leaves, which are mainly useful for the construction
of template meta programs.

Finally, there are mixin implementations of the three default node concepts in
the library (leaf nodes, power nodes and composite nodes). For more complicated
use patterns, the library also contains generic implementations of proxy nodes
and, on top of that functionality, filtered nodes that can reorder and / or
restrict access to some of their children.

The TypeTree library was originally developed as part of [PDELab][0] to support its
tree-based abstraction of function spaces, but has attracted wider-spread interest.
To facility integration with other projects, we have extracted the code from
PDELab into this standalone library

This package contains the TypeTree library code. For usage examples your best bet
is to look at the PDELab package.

If you have downloaded a release tarball, you can find the autogenerated Doxygen
API documentation in doc/doxygen/html. Otherwise, you can build this documentation
yourself by calling "make doc". Note that you need Doxygen and GraphViz available at
configure time to be able to build the documentation.

If you need help, please ask on the [PDELab mailinglist][5] for now. Bugs can be
submitted to the [bugtracker][6] instead.

See the file [CHANGELOG.md][8] for recent changes to the library.

Dependencies
------------

TypeTree depends on the following software packages:

* The dune-common library from DUNE, version 2.6.0. The dependency is actually very
  weak, and you can easily use the library standalone by replacing the few convenience
  components reused from dune-common (and by replacing the build system, of course).

* TypeTree uses lots and lots of templates, so you need a decent C++ compiler.
  This release requires a compiler that is at least compatible with GCC 5 in C++14
  mode, so it whould work with all recent versions of GCC since version 5 and modern versions
  of the clang compiler (5+). Very recent versions of ICC (icpc 18+) should also work, but
  this is not tested on a regular basis.

License
-------

The TypeTree library, headers and test programs are free open-source software,
dual-licensed under version 3 or later of the GNU Lesser General Public License
and version 2 of the GNU General Public License with a special run-time exception.

See the file [LICENSE.md][7] for full copying permissions.

Installation
------------

For installation instructions please see the [DUNE website][2].

Links
-----

[0]: http://www.dune-project.org/pdelab/
[1]: http://www.dune-project.org
[2]: http://www.dune-project.org/doc/installation-notes.html
[4]: http://gcc.gnu.org/onlinedocs/libstdc++/faq.html#faq.license
[5]: http://lists.dune-project.org/mailman/listinfo/dune-pdelab
[6]: http://gitlab.dune-project.org/pdelab/dune-typetree/issues
[7]: LICENSE.md
[8]: CHANGELOG.md
