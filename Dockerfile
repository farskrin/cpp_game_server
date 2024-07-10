FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip install conan==1.*


COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    #conan install .. --build=missing \
    conan install .. -s compiler.libcxx=libstdc++11 -s build_type=Debug --build=missing

COPY ./src /app/src
COPY ./tests /app/tests
COPY ./data /app/data
COPY ./static /app/static
COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    cmake --build . 


#-----------------
FROM ubuntu:22.04 as run 
RUN groupadd -r www && useradd -r -g www www
USER www 

COPY --from=build /app/build/game_server /app/
COPY ./data /app/data
COPY ./static /app/static

#ENV GAME_DB_URL "postgres://postgres:Immodest@localhost:30432/game_db1"

ENTRYPOINT ["/app/game_server", "--config-file", "/app/data/config.json", "--www-root", "/app/static"]

