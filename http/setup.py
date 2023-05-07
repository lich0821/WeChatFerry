#! /usr/bin/env python3
# -*- coding: utf-8 -*-


from __future__ import print_function
from setuptools import setup, find_packages

import wcfhttp

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()


setup(
    name="wcfhttp",
    version=wcfhttp.core.__version__,
    author="Changhua",
    author_email="lichanghua0821@gmail.com",
    description="一个玩微信的工具",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="MIT",
    url="https://github.com/lich0821/WeChatFerry",
    python_requires=">=3.8",
    packages=find_packages(),
    include_package_data=True,
    entry_points={
        'console_scripts': [
            'wcfhttp=wcfhttp.main:main'
        ]
    },
    install_requires=[
        "setuptools",
        f"wcferry=={wcfhttp.core.__version__}",
        "fastapi",
        "uvicorn[standard]"
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
