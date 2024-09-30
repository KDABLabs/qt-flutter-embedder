import 'dart:async';
import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

final GlobalObjectKey<MyOrangeContainerState> myWidgetKey =
    GlobalObjectKey<MyOrangeContainerState>('myWidgetKey');

void main() async {
  runWidget(ViewCollection(views: [
    View(
      view: PlatformDispatcher.instance.implicitView!,
      child: const MyApp(),
    ),
    View(
      view: PlatformDispatcher.instance.views.where((v) => v.viewId == 1).first,
      child: MyOrangeContainer(key: myWidgetKey),
    )
  ]));

  const platform = MethodChannel('qtembedder.kdab.com/testPlatformChannel');
  Timer t = Timer(const Duration(seconds: 2), () async {
    print("Trying to call platform channel");
    try {
      final result = await platform.invokeMethod<int>('testPlatformChannel');
      print("Platform channel returned $result");
    } on PlatformException catch (e) {
      print("Failed to call platform method: '${e.message}'.");
    }
    print("Finished trying to call platform channel");
  });
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class MyOrangeContainer extends StatefulWidget {
  MyOrangeContainer({super.key});

  @override
  State<StatefulWidget> createState() {
    return MyOrangeContainerState();
  }
}

class MyOrangeContainerState extends State<MyOrangeContainer> {
  int _window2Counter = 0;
  void incrementCounter() {
    setState(() {
      _window2Counter++;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        title: 'Window #2',
        theme: ThemeData(
          colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
          useMaterial3: true,
        ),
        home: Container(
          color: Colors.orange,
          child: Center(
            child: Text(
              "${_window2Counter}",
              style: Theme.of(context).textTheme.headlineMedium,
            ),
          ),
        ));
  }
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;

  void _incrementCounter() {
    setState(() {
      _counter++;
    });

    var state = myWidgetKey.currentState;
    if (state != null) {
      state.incrementCounter();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'You have pushed the button this many times:',
            ),
            Text(
              '$_counter',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Increment',
        child: const Icon(Icons.add),
      ),
    );
  }
}
