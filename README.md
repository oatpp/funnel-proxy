# Funnel-Proxy

Transparent multiplexing proxy.  
It can handle hundreds of thousand connections and proxy them to downstream services using a fixed connection pool.  
It can also be embedded in Oat++ services via [monolithization](https://oatpp.io/docs/monolithization/).

See more:

- [Oat++ Website](https://oatpp.io/)
- [Oat++ Github Repository](https://github.com/oatpp/oatpp)
- [Oat++ Monolithization](https://oatpp.io/docs/monolithization/)

## When To Use

- You have a simple request-response API and you want to increase the possible concurrency level it can run on.

## When Not To Use

- Streaming. You can't do streaming through funnel-proxy, because the stream will acquire a connection from connection-pool
forever and you'll limit your concurrency level to the number of connections in the connection pool.  
*However you can implement your custom streaming on the proxy-side - when proxy doing continuous streaming while requesting chunk-by-chunk from the source service*
- Long-polling - same as for streaming.
