#!/bin/bash
docker run --rm -it -v "${PWD}:/code" -w "/code" --name "compilers_env" compilers_image