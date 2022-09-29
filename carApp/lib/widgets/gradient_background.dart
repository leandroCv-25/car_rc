import 'package:flutter/material.dart';

class GradientBackGround extends StatelessWidget {
  final Color? endColor;
  final Color? beginColor;

  const GradientBackGround({super.key, this.beginColor, this.endColor});

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
          gradient: LinearGradient(
              begin: Alignment.topCenter,
              end: Alignment.bottomCenter,
              colors: [
            beginColor ?? Theme.of(context).primaryColorDark,
            endColor ?? Theme.of(context).primaryColorLight,
          ],
              stops: const [
            0.0,
            1.0
          ])),
    );
  }
}
