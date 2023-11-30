##
# Copyright 2009-2023 Ghent University
#
# This file is part of EasyBuild,
# originally created by the HPC team of Ghent University (http://ugent.be/hpc/en),
# with support of Ghent University (http://ugent.be/hpc),
# the Flemish Supercomputer Centre (VSC) (https://www.vscentrum.be),
# Flemish Research Foundation (FWO) (http://www.fwo.be/en)
# and the Department of Economy, Science and Innovation (EWI) (http://www.ewi-vlaanderen.be/en).
#
# https://github.com/easybuilders/easybuild
#
# EasyBuild is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation v2.
#
# EasyBuild is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EasyBuild.  If not, see <http://www.gnu.org/licenses/>.
##
"""
EasyBuild support for software that uses the GNU installation procedure,
i.e. configure/make/make install, implemented as an easyblock.

@author: Stijn De Weirdt (Ghent University)
@author: Dries Verdegem (Ghent University)
@author: Kenneth Hoste (Ghent University)
@author: Pieter De Baets (Ghent University)
@author: Jens Timmerman (Ghent University)
@author: Toon Willems (Ghent University)
@author: Maxime Boissonneault (Compute Canada - Universite Laval)
@author: Alan O'Cais (Juelich Supercomputing Centre)
"""
import os
import re
import stat
from datetime import datetime

from easybuild.base import fancylogger
from easybuild.easyblocks import VERSION as EASYBLOCKS_VERSION
from easybuild.framework.easyblock import EasyBlock
from easybuild.framework.easyconfig import CUSTOM
from easybuild.tools.build_log import print_warning
from easybuild.tools.config import source_paths, build_option
from easybuild.tools.filetools import CHECKSUM_TYPE_SHA256, adjust_permissions, compute_checksum, download_file
from easybuild.tools.filetools import read_file, remove_file
from easybuild.tools.py2vs3 import string_type
from easybuild.tools.run import run_cmd

# string that indicates that a configure script was generated by Autoconf
# note: bytes string since this constant is used to check the contents of 'configure' which is read as bytes
# (mainly important when EasyBuild is using Python 3)
AUTOCONF_GENERATED_MSG = b"Generated by GNU Autoconf"

# download location & SHA256 for config.guess script
CONFIG_GUESS_VERSION = '2018-08-29'
CONFIG_GUESS_URL_STUB = "https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb="
CONFIG_GUESS_COMMIT_ID = "59e2ce0e6b46bb47ef81b68b600ed087e14fdaad"
CONFIG_GUESS_SHA256 = "c02eb9cc55c86cfd1e9a794e548d25db5c9539e7b2154beb649bc6e2cbffc74c"


DEFAULT_BUILD_CMD = 'make'
DEFAULT_BUILD_TARGET = ''
DEFAULT_CONFIGURE_CMD = './configure'
DEFAULT_INSTALL_CMD = 'make install'
DEFAULT_TEST_CMD = 'make'


def check_config_guess(config_guess):
    """Check timestamp & SHA256 checksum of config.guess script.

    :param config_guess: Path to config.guess script to check
    :return: Whether the script is valid (matches the version and checksum)
    """
    log = fancylogger.getLogger('check_config_guess')

    # config.guess includes a "timestamp='...'" indicating the version
    config_guess_version = None
    version_regex = re.compile("^timestamp='(.*)'", re.M)
    res = version_regex.search(read_file(config_guess))
    if res:
        config_guess_version = res.group(1)

    config_guess_checksum = compute_checksum(config_guess, checksum_type=CHECKSUM_TYPE_SHA256)
    try:
        config_guess_timestamp = datetime.fromtimestamp(os.stat(config_guess).st_mtime).isoformat()
    except OSError as err:
        log.warning("Failed to determine timestamp of %s: %s", config_guess, err)
        config_guess_timestamp = None

    log.info("config.guess version: %s (last updated: %s, SHA256 checksum: %s)",
             config_guess_version, config_guess_timestamp, config_guess_checksum)

    result = True

    # check version & SHA256 checksum before declaring victory
    if config_guess_version != CONFIG_GUESS_VERSION:
        result = False
        log.warning("config.guess version at %s does not match expected version: %s vs %s",
                    config_guess, config_guess_version, CONFIG_GUESS_VERSION)

    elif config_guess_checksum != CONFIG_GUESS_SHA256:
        result = False
        log.warning("SHA256 checksum of config.guess at %s does not match expected checksum: %s vs %s",
                    config_guess, config_guess_checksum, CONFIG_GUESS_SHA256)

    return result


def obtain_config_guess(download_source_path=None, search_source_paths=None):
    """
    Locate or download an up-to-date config.guess

    :param download_source_path: Path to download config.guess to
    :param search_source_paths: Paths to search for config.guess
    :return: Path to config.guess or None
    """
    log = fancylogger.getLogger('obtain_config_guess')

    eb_source_paths = source_paths()

    if download_source_path is None:
        download_source_path = eb_source_paths[0]
    else:
        log.deprecated("Specifying custom source path to download config.guess via 'download_source_path'", '5.0')

    if search_source_paths is None:
        search_source_paths = eb_source_paths
    else:
        log.deprecated("Specifying custom location to search for updated config.guess via 'search_source_paths'", '5.0')

    config_guess = 'config.guess'
    sourcepath_subdir = os.path.join('generic', 'eb_v%s' % EASYBLOCKS_VERSION, 'ConfigureMake')

    config_guess_path = None

    # check if config.guess has already been downloaded to source path
    for path in search_source_paths:
        cand_config_guess_path = os.path.join(path, sourcepath_subdir, config_guess)
        if os.path.isfile(cand_config_guess_path) and check_config_guess(cand_config_guess_path):
            force_download = build_option('force_download')
            if force_download:
                print_warning("Found file %s at %s, but re-downloading it anyway..."
                              % (config_guess, cand_config_guess_path))
            else:
                config_guess_path = cand_config_guess_path
                log.info("Found %s at %s", config_guess, config_guess_path)
            break

    if not config_guess_path:
        cand_config_guess_path = os.path.join(download_source_path, sourcepath_subdir, config_guess)
        config_guess_url = CONFIG_GUESS_URL_STUB + CONFIG_GUESS_COMMIT_ID
        if not download_file(config_guess, config_guess_url, cand_config_guess_path):
            print_warning("Failed to download recent %s to %s", config_guess, cand_config_guess_path, log=log)
        elif not check_config_guess(cand_config_guess_path):
            print_warning("Verification failed for file %s, not using it!", cand_config_guess_path, log=log)
            remove_file(cand_config_guess_path)
        else:
            config_guess_path = cand_config_guess_path
            adjust_permissions(config_guess_path, stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH, add=True)
            log.info("Verified %s at %s, using it if required", config_guess, config_guess_path)

    return config_guess_path


class ConfigureMake(EasyBlock):
    """
    Support for building and installing applications with configure/make/make install
    """

    @staticmethod
    def extra_options(extra_vars=None):
        """Extra easyconfig parameters specific to ConfigureMake."""
        extra_vars = EasyBlock.extra_options(extra=extra_vars)
        extra_vars.update({
            'build_cmd': [DEFAULT_BUILD_CMD, "Build command to use", CUSTOM],
            'build_cmd_targets': [DEFAULT_BUILD_TARGET, "Target name (string) or list of target names to build",
                                  CUSTOM],
            'build_type': [None, "Value to provide to --build option of configure script, e.g., x86_64-pc-linux-gnu "
                                 "(determined by config.guess shipped with EasyBuild if None,"
                                 " False implies to leave it up to the configure script)", CUSTOM],
            'configure_cmd': [DEFAULT_CONFIGURE_CMD, "Configure command to use", CUSTOM],
            'configure_cmd_prefix': ['', "Prefix to be glued before ./configure", CUSTOM],
            'configure_without_installdir': [False, "Avoid passing an install directory to the configure command "
                                                    "(such as via --prefix)", CUSTOM],
            'host_type': [None, "Value to provide to --host option of configure script, e.g., x86_64-pc-linux-gnu "
                                "(determined by config.guess shipped with EasyBuild if None,"
                                " False implies to leave it up to the configure script)", CUSTOM],
            'install_cmd': [DEFAULT_INSTALL_CMD, "Install command to use", CUSTOM],
            'prefix_opt': [None, "Prefix command line option for configure script ('--prefix=' if None)", CUSTOM],
            'tar_config_opts': [False, "Override tar settings as determined by configure.", CUSTOM],
            'test_cmd': [None, "Test command to use ('runtest' value is appended, default: '%s')" % DEFAULT_TEST_CMD,
                         CUSTOM],
        })
        return extra_vars

    def __init__(self, *args, **kwargs):
        """Initialize easyblock."""
        super(ConfigureMake, self).__init__(*args, **kwargs)

        self.config_guess = None

    def obtain_config_guess(self, download_source_path=None, search_source_paths=None):
        """
        Locate or download an up-to-date config.guess for use with ConfigureMake

        :param download_source_path: Path to download config.guess to
        :param search_source_paths: Paths to search for config.guess
        :return: Path to config.guess or None
        """
        return obtain_config_guess(download_source_path=download_source_path, search_source_paths=search_source_paths)

    def check_config_guess(self):
        """Check timestamp & SHA256 checksum of config.guess script."""
        # log version, timestamp & SHA256 checksum of config.guess that was found (if any)
        if self.config_guess:
            check_config_guess(self.config_guess)

    def fetch_step(self, *args, **kwargs):
        """Custom fetch step for ConfigureMake so we use an updated config.guess."""
        super(ConfigureMake, self).fetch_step(*args, **kwargs)

        # Use an updated config.guess from a global location (if possible)
        self.config_guess = self.obtain_config_guess()

    def determine_build_and_host_type(self):
        """
        Return the resolved build and host type for use with --build and --host
        Uses the EasyConfig values or queries config.guess if those are not set
        Might return None for either value
        """
        build_type = self.cfg.get('build_type')
        host_type = self.cfg.get('host_type')

        if build_type is None or host_type is None:
            # config.guess script may not be obtained yet despite the call in fetch_step,
            # for example when installing a Bundle component with ConfigureMake
            if not self.config_guess:
                self.config_guess = self.obtain_config_guess()

            if not self.config_guess:
                print_warning("No config.guess available, not setting '--build' option for configure step\n"
                              "EasyBuild attempts to download a recent config.guess but seems to have failed!")
            else:
                self.check_config_guess()
                system_type, _ = run_cmd(self.config_guess, log_all=True)
                system_type = system_type.strip()
                self.log.info("%s returned a system type '%s'", self.config_guess, system_type)

                if build_type is None:
                    build_type = system_type
                    self.log.info("Providing '%s' as value to --build option of configure script", build_type)

                if host_type is None:
                    host_type = system_type
                    self.log.info("Providing '%s' as value to --host option of configure script", host_type)

        return build_type, host_type

    def configure_step(self, cmd_prefix=''):
        """
        Configure step
        - typically ./configure --prefix=/install/path style
        """

        if self.cfg.get('configure_cmd_prefix'):
            if cmd_prefix:
                tup = (cmd_prefix, self.cfg['configure_cmd_prefix'])
                self.log.debug("Specified cmd_prefix '%s' is overruled by configure_cmd_prefix '%s'" % tup)
            cmd_prefix = self.cfg['configure_cmd_prefix']

        if self.cfg.get('tar_config_opts'):
            # setting am_cv_prog_tar_ustar avoids that configure tries to figure out
            # which command should be used for tarring/untarring
            # am__tar and am__untar should be set to something decent (tar should work)
            tar_vars = {
                'am__tar': 'tar chf - "$$tardir"',
                'am__untar': 'tar xf -',
                'am_cv_prog_tar_ustar': 'easybuild_avoid_ustar_testing'
            }
            for (key, val) in tar_vars.items():
                self.cfg.update('preconfigopts', "%s='%s'" % (key, val))

        prefix_opt = self.cfg.get('prefix_opt')
        if prefix_opt is None:
            prefix_opt = '--prefix='

        configure_command = cmd_prefix + (self.cfg.get('configure_cmd') or DEFAULT_CONFIGURE_CMD)

        # avoid using config.guess from an Autoconf generated package as it is frequently out of date;
        # use the version downloaded by EasyBuild instead, and provide the result to the configure command;
        # it is possible that the configure script is generated using preconfigopts...
        # if so, we're at the mercy of the gods
        build_and_host_options = []

        # note: reading contents of 'configure' script in bytes mode,
        # to avoid problems when non-UTF-8 characters are included
        # see https://github.com/easybuilders/easybuild-easyblocks/pull/1817
        if os.path.exists(configure_command) and AUTOCONF_GENERATED_MSG in read_file(configure_command, mode='rb'):
            build_type, host_type = self.determine_build_and_host_type()
            if build_type:
                build_and_host_options.append(' --build=' + build_type)
            if host_type:
                build_and_host_options.append(' --host=' + host_type)

        if self.cfg.get('configure_without_installdir'):
            configure_prefix = ''
            if self.cfg.get('prefix_opt'):
                print_warning("Specified prefix_opt '%s' is ignored due to use of configure_without_installdir",
                              prefix_opt)
        else:
            configure_prefix = prefix_opt + self.installdir

        cmd = ' '.join(
            [
                self.cfg['preconfigopts'],
                configure_command,
                configure_prefix,
            ] + build_and_host_options + [self.cfg['configopts']]
        )

        (out, _) = run_cmd(cmd, log_all=True, simple=False)

        return out

    def build_step(self, verbose=False, path=None):
        """
        Start the actual build
        - typical: make -j X
        """

        paracmd = ''
        if self.cfg['parallel']:
            paracmd = "-j %s" % self.cfg['parallel']

        targets = self.cfg.get('build_cmd_targets') or DEFAULT_BUILD_TARGET
        # ensure strings are converted to list
        targets = [targets] if isinstance(targets, string_type) else targets

        for target in targets:
            cmd = ' '.join([
                self.cfg['prebuildopts'],
                self.cfg.get('build_cmd') or DEFAULT_BUILD_CMD,
                target,
                paracmd,
                self.cfg['buildopts'],
            ])
            self.log.info("Building target '%s'", target)

            (out, _) = run_cmd(cmd, path=path, log_all=True, simple=False, log_output=verbose)

        return out

    def test_step(self):
        """
        Test the compilation
        - default: None
        """

        test_cmd = self.cfg.get('test_cmd') or DEFAULT_TEST_CMD
        runtest = self.cfg['runtest']
        if runtest or test_cmd != DEFAULT_TEST_CMD:
            # Make run_test a string (empty if it is e.g. a boolean)
            if not isinstance(runtest, string_type):
                runtest = ''
            # Compose command filtering out empty values
            cmd = ' '.join([x for x in (self.cfg['pretestopts'], test_cmd, runtest, self.cfg['testopts']) if x])
            (out, _) = run_cmd(cmd, log_all=True, simple=False)

            return out

    def install_step(self):
        """
        Create the installation in correct location
        - typical: make install
        """

        cmd = ' '.join([
            self.cfg['preinstallopts'],
            self.cfg.get('install_cmd') or DEFAULT_INSTALL_CMD,
            self.cfg['installopts'],
        ])

        (out, _) = run_cmd(cmd, log_all=True, simple=False)

        return out