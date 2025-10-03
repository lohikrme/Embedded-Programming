# nodered/node-red:2.2.3-12 (close to this version)
FROM nodered/node-red:latest

LABEL Description="Node-Red container"

USER root

ENV HOME /root

# install sqlite3 support to container
RUN cd /root && apk update && apk add sqlite

USER node-red
RUN cd /usr/src/node-red

# Note! Do not use CMD in node-red container, will not work!
# CMD /bin/bash
