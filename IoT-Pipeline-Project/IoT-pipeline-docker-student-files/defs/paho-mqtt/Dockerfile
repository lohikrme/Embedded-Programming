# syntax=docker/dockerfile:1
# Tämä rivi kertoo Dockerille, että käytetään modernia Dockerfile-syntaksia (versio 1).
# Ei ole pakollinen, mutta suositeltava uusissa projekteissa.

# Määritellään Ubuntu-version argumentti, jotta sitä voi vaihtaa helposti buildin yhteydessä.
ARG UBUNTU_VERSION=24.04

# Käytetään valittua Ubuntu-versiota base imagena.
FROM ubuntu:${UBUNTU_VERSION}

# Näytetään buildin aikana, mikä Ubuntu-versio on valittu.
ARG UBUNTU_VERSION
RUN echo "Selected Ubuntu version: ${UBUNTU_VERSION}"

# Lisätään kuvaileva label, joka kertoo kontin tarkoituksen.
LABEL Description="Build environment server"

# Asetetaan HOME-ympäristömuuttuja root-käyttäjälle.
ENV HOME=/root

# Määritellään työskentelyhakemisto kontissa.
# Tänne kopioidaan lähdekoodit ja kirjastot.
WORKDIR /usr/src/

# Asennetaan kehitystyökalut ja kirjastot:
# - build-essential: gcc/g++ ja make
# - clang: vaihtoehtoinen C/C++-kääntäjä
# - gdb: debugger
# - wget, curl, unzip: lataustyökalut
# - nano, tree: apuohjelmat
# - valgrind: muistivuotojen tarkistus
# - git: versionhallinta
# - autoconf, automake, libtool: build-työkalut
# - libssl-dev: TLS/SSL-tuki
# - ssh: mahdollistaa yhteydet
# - pkg-config: kirjastojen hallinta
# - ca-certificates: varmistaa HTTPS-yhteydet
# - libcjson-dev: JSON-kirjasto
# - doxygen, graphviz: dokumentaation generointiin
# - cmake needed to build c code
# - libc-bin needed so that ldconfig is available to update dynamic linker cache
RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    gdb \
    wget \
    nano \
    tree \
    valgrind \
    git \
    autoconf \
    automake \
    libtool \
    curl \
    unzip \
    libssl-dev \
    ssh \
    pkg-config \
    ca-certificates \
    libcjson-dev \
    doxygen \
    graphviz \
    cmake \
    libc-bin && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Tulostetaan buildin lopuksi viesti, jotta näkee että kaikki meni läpi.
RUN echo "Build complete for C++ development environment running on ubuntu ${UBUNTU_VERSION}..."

# CMD pitää kontin hengissä. Ilman tätä kontti sulkeutuisi heti.
# "sleep infinity" on yleinen tapa pitää kehityskontti käynnissä.
CMD ["sleep","infinity"]
