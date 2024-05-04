module.exports = {
  devServer: {
    port: 8081,
  },
  configureWebpack: (x) => {
    x.optimization = {
      splitChunks: {
        chunks: "all",
        minSize: 0,
        maxSize: 3000000,
        minChunks: 1,
        maxAsyncRequests: 1
      },
    };
  },
};
