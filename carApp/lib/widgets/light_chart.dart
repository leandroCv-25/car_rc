//flutter
import 'package:flutter/material.dart';

class LightChart extends StatelessWidget {
  final bool isLightOn;

  const LightChart(this.isLightOn, {super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: ((context, constraint) {
        return Icon(
          Icons.lightbulb_outline,
          color: isLightOn ? Colors.yellow : Colors.grey,
          size: constraint.maxHeight * 0.6,
        );
      }),
    );
  }
}
