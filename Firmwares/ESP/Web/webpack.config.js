const path = require("path");
const webpack = require("webpack");
const CleanWebpackPlugin = require("clean-webpack-plugin");
const CompressionPlugin = require("compression-webpack-plugin");
const ExtractTextPlugin = require("extract-text-webpack-plugin");
const PostCSSAssetsPlugin = require("postcss-assets-webpack-plugin");
const CopyWebpackPlugin = require("copy-webpack-plugin");

module.exports = function (env) {
  const outputPath = path.join(__dirname, "../data");
  const exports = {
    cache: true,
    entry: {
      main: "./src/index.jsx"
    },
    output: {
      path: outputPath,
      filename: "[name].js",
      publicPath: "/"
    },
    plugins: [
      new CleanWebpackPlugin(path.join(outputPath, "*"), {root: path.join(__dirname, ".."), verbose: false}),
      new webpack.NoEmitOnErrorsPlugin(),
      new ExtractTextPlugin("style.css"),
      new PostCSSAssetsPlugin({
        test: /\.css$/,
        log: false,
        plugins: [
          require("precss"),
          require("autoprefixer")
        ],
      })
    ],
    module: {
      rules: [
        {
          test: /\.(jsx|js)$/,
          loader: "babel-loader",
          include: [path.resolve(__dirname, "src")],
          exclude: /(node_modules|bower_components)/,
          options: {cacheDirectory: path.resolve(__dirname, "babel_cache/")}
        },
        {
          test: /\.scss$/,
          use: ExtractTextPlugin.extract({
            fallback: "style-loader",
            use: ["css-loader", "sass-loader"]
          })
        },
        {
          test: /\.(json)$/,
          loader: "json-loader"
        }
      ]
    }
  };

  const copyTargets = [{from: 'src/static/', to: './'}];
  exports.plugins.push(new CopyWebpackPlugin(copyTargets));

  if (env && env.dev) {
    // Смена путей в index.html на локальный сервер
    exports.devtool = 'cheap-module';
    copyTargets.push({
      from: '../server.h', to: '../server.h',
      transform: content => {
        content = content.toString();
        content = content.replace('href=\\"style.css\\"', 'href=\\"http://localhost:9080/style.css\\"');
        content = content.replace('src=\\"main.js\\"', 'src=\\"http://localhost:9080/main.js\\"');
        return content;
      }
    });
  } else {
    // Смена путей на выпуск
    exports.devtool = 'cheap-module';
    copyTargets.push({
      from: '../server.h', to: '../server.h',
      transform: content => {
        content = content.toString();
        content = content.replace('href=\\"http://localhost:9080/style.css\\"', 'href=\\"style.css\\"');
        content = content.replace('src=\\"http://localhost:9080/main.js\\"', 'src=\\"main.js\\"');
        return content;
      }
    });
  }

  if (env && env.compress) {
    // Сжатие всех файлов проекта
    exports.plugins.push(new CompressionPlugin({
      asset: "[path].gz[query]",
      algorithm: "gzip",
      test: /\.(js|css|json)$/,
      threshold: 0,
      minRatio: 1,
      deleteOriginalAssets: true
    }));
  }

  return exports;
};
