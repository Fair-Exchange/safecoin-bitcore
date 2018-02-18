FROM kolobus/ubuntu:safecoin
MAINTAINER Mihail Fedorov <tech@fedorov.net>

ADD ./ /safecoin
ENV HOME /safecoin
WORKDIR /safecoin

# configure || true or it WILL halt
RUN cd /safecoin && \
    ./autogen.sh && \
    ./configure --with-incompatible-bdb --with-gui || true && \
    ./zcutil/build.sh -j$(nproc)

# Unknown stuff goes here

RUN ln -sf /safecoin/src/safecoind /usr/bin/safecoind && \
    ln -sf /safecoin/zcutil/docker-entrypoint.sh /usr/bin/entrypoint && \
    ln -sf /safecoin/zcutil/docker-safecoin-cli.sh /usr/bin/safecoin-cli

CMD ["entrypoint"]
