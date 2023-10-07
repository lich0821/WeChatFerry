#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import wcfauto
from setuptools import find_packages, setup

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()


setup(
    name="wcfauto",
    version=wcfauto.__version__,
    author="bujinzhang",
    author_email="",
    description="一个玩微信的工具",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="MIT",
    url="https://github.com/lich0821/WeChatFerry",
    python_requires=">=3.8",
    packages=find_packages(),
    include_package_data=True,
    install_requires=[
        "setuptools",
        "wcferry",
    ],
    classifiers=[
        "Environment :: Win32 (MS Windows)",
        "Intended Audience :: Developers",
        "Intended Audience :: Customer Service",
        "Topic :: Communications :: Chat",
        "Operating System :: Microsoft :: Windows",
        "Programming Language :: Python",
    ],
    project_urls={
        "Documentation": "https://wechatferry.readthedocs.io/zh/latest/index.html",
        "GitHub": "https://github.com/lich0821/WeChatFerry/",
    },
)
