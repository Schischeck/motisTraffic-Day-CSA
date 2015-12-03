var path = require('path');
var webpack = require('webpack');
var HtmlWebpackPlugin = require('html-webpack-plugin')

module.exports = {
  devtool: 'eval',
  entry: [
    './src/index'
  ],
  output: {
    path: path.join(__dirname, 'dist/'),
    filename: 'bundle.js',
    publicPath: ''
  },
  plugins: [
    new HtmlWebpackPlugin({
      title: "MOTIS",
      filename: "index.html"
    }),
    new webpack.optimize.OccurenceOrderPlugin(),
    new webpack.DefinePlugin({
      'process.env': {
        'NODE_ENV': JSON.stringify('production')
      }
    }),
    new webpack.optimize.UglifyJsPlugin({
      compressor: {
        warnings: false
      }
    })
  ],
  module: {
    loaders: [{
      test: /\.js$/,
      loaders: ['babel'],
      include: path.join(__dirname, 'src')
    },
    {
      test: /\.scss$/,
      loader: 'style!css?modules!sass'
    },
    {
      test: /\.css$/,
      loader: 'style!css'
    },
    {
      test: /\.(png|woff|woff2|eot|ttf|svg)$/,
      loader: 'url-loader?limit=100000'
    }]
  }
};
